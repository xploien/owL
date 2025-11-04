
#include "Reproductor.hpp"
#include <iostream>

// Cola global
MessageQueue globalQueue;

void MessageQueue::push(Command cmd)
{
  std::lock_guard<std::mutex> lock(mutex_);
  queue_.push(cmd);
  cv_.notify_one();
}

Command MessageQueue::pop()
{
  std::unique_lock<std::mutex> lock(mutex_);
  cv_.wait(lock, [&] {
    return !queue_.empty() || stop_;
  });
  if (queue_.empty())
    return nullptr;
  auto cmd = queue_.front();
  queue_.pop();
  return cmd;
}

void MessageQueue::stop()
{
  {
    std::lock_guard<std::mutex> lock(mutex_);
    stop_ = true;
  }
  cv_.notify_all();
}

// cola de info:
InfoQueue globalInfoQueue;

// ---------------------------
// InfoQueue implementación
// ---------------------------
void InfoQueue::update(const MusicInfo &info)
{
  std::lock_guard<std::mutex> lock(mutex_);
  info_ = info;
  has_update_ = true;
}

MusicInfo InfoQueue::get()
{
  std::lock_guard<std::mutex> lock(mutex_);
  has_update_ = false;
  return info_;
}

bool InfoQueue::hasUpdate()
{
  std::lock_guard<std::mutex> lock(mutex_);
  return has_update_;
}
// ---------------------------
// Reproductor
// ---------------------------
Reproductor::Reproductor(VLC::Instance &instance)
  : player(instance), estado(new Paused())
{}

Reproductor::~Reproductor()
{
  delete estado;
}

void Reproductor::setEstado(Estado *nuevo)
{
  delete estado;
  estado = nuevo;
}

void Reproductor::play()
{
  estado->play(this);
  enviarInfo(); // Enviar actualización cuando cambia el estado
}
void Reproductor::pause()
{
  estado->pause(this);
  enviarInfo(); // Enviar actualización cuando cambia el estado
}
std::string Reproductor::getEstado() const
{
  return estado->nombre();
}

void Reproductor::enviarInfo()
{
  MusicInfo info;

  // Obtener estado
  info.estado = getEstado();

  // Obtener información del media actual
  auto media = player.media();
  if (media)
    {
      info.title = media->meta(libvlc_meta_Title);
      info.artist = media->meta(libvlc_meta_Artist);
      info.album = media->meta(libvlc_meta_Album);
      info.genre = media->meta(libvlc_meta_Genre);
      info.artworkUrl = media->meta(libvlc_meta_ArtworkURL);
    }

  // Obtener posición y duración
  info.position = player.position() * player.length() / 1000.0f; // en segundos
  info.duration = player.length() / 1000.0f;                     // en segundos

  info.has_update = true;

  // Enviar a la cola de información
  globalInfoQueue.update(info);
}

// ---------------------------
// Estados
// ---------------------------
void Playing::play(Reproductor *r)
{
  std::cout << "Ya se está reproduciendo\n";
}

void Playing::pause(Reproductor *r)
{
  r->player.pause();
  r->setEstado(new Paused());
  std::cout << "⏸ Pausado\n";
  auto mediaPtr = r->player.media();
}

void Paused::play(Reproductor *r)
{
  r->player.play();
  r->setEstado(new Playing());
  std::cout << "▶ Reproduciendo\n";
}

void Paused::pause(Reproductor *r)
{
  std::cout << "Ya está en pausa\n";
}

// ---------------------------
// MusicServer
// ---------------------------
int MusicServer(int argc, char *argv[])
{
  if (argc < 2)
    {
      std::cerr << "Uso: " << argv[0] << " archivo.mp3\n";
      return 1;
    }

  try
    {
      const char *vlc_args[] = {"--no-xlib", "--quiet"};
      VLC::Instance instance(2, vlc_args);

      Reproductor reproductor(instance);
      VLC::Media media(instance, argv[1], VLC::Media::FromPath);
      reproductor.player.setMedia(media);

      media.parseWithOptions(VLC::Media::ParseFlags::Local, -1);
      // IMPORTANTE: Esperar a que termine el parsing
      // Método 1: Esperar con un timeout
      int timeout = 5000; // 5 segundos máximo
      int elapsed = 0;
      while (media.parsedStatus() != VLC::Media::ParsedStatus::Done && elapsed < timeout)
        {
          std::this_thread::sleep_for(std::chrono::milliseconds(100));
          elapsed += 100;
        }

      // Verificar si el parsing fue exitoso
      if (media.parsedStatus() == VLC::Media::ParsedStatus::Done)
        {
          std::cout << "✓ Metadatos parseados correctamente\n";
        }
      else
        {
          std::cout << "⚠ Advertencia: No se pudieron parsear completamente los metadatos\n";
        }

      // Ahora sí obtener los metadatos
      auto title = media.meta(libvlc_meta_Title);
      if (!title.empty())
        std::cout << "Título: " << title << "\n";

      auto artist = media.meta(libvlc_meta_Artist);
      if (!artist.empty())
        std::cout << "Artista: " << artist << "\n";

      // Enviar información inicial
      reproductor.enviarInfo();

      bool running = true;

      // Timer para actualizar info periódicamente
      auto lastUpdate = std::chrono::steady_clock::now();

      while (running)
        {
          auto cmd = globalQueue.pop();
          if (cmd)
            {
              cmd(reproductor, running);
            }
          // Actualizar info cada segundo si está reproduciendo
          auto now = std::chrono::steady_clock::now();
          auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastUpdate).count();

          if (elapsed >= 1000 && reproductor.getEstado() == "Playing")
            {
              reproductor.enviarInfo();
              lastUpdate = now;
            }
        }
    }
  catch (const std::exception &e)
    {
      std::cerr << "Error: " << e.what() << "\n";
      return 1;
    }

  return 0;
}
