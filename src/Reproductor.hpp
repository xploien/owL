
#pragma once

#include "../vlcpp/vlc.hpp"
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <string>

// Forward declarations
class Reproductor;
class Estado;

// ---------------------------
// Cola de mensajes
// ---------------------------
using Command = std::function<void(Reproductor &, bool &)>;

class MessageQueue
{
public:
  void push(Command cmd);
  Command pop();
  void stop();

private:
  std::queue<Command> queue_;
  std::mutex mutex_;
  std::condition_variable cv_;
  bool stop_ = false;
};

// Cola global (declaración)
extern MessageQueue globalQueue;

// ---------------------------
// Cola de información (datos desde MusicServer)
// ---------------------------
struct MusicInfo
{
  std::string title;
  std::string artist;
  std::string album;
  std::string genre;
  std::string artworkUrl;
  std::string estado;
  float position; // posición actual en segundos
  float duration; // duración total en segundos
  bool has_update;
};

class InfoQueue
{
public:
  void update(const MusicInfo &info);
  MusicInfo get();
  bool hasUpdate();

private:
  MusicInfo info_;
  std::mutex mutex_;
  bool has_update_ = false;
};

// Cola de información global
extern InfoQueue globalInfoQueue;
// ---------------------------
// Estados
// ---------------------------
class Estado
{
public:
  virtual void play(Reproductor *reproductor) = 0;
  virtual void pause(Reproductor *reproductor) = 0;
  virtual std::string nombre() const = 0;
  virtual ~Estado()
  {}
};

class Playing : public Estado
{
public:
  void play(Reproductor *reproductor) override;
  void pause(Reproductor *reproductor) override;
  std::string nombre() const override
  {
    return "Playing";
  }
};

class Paused : public Estado
{
public:
  void play(Reproductor *reproductor) override;
  void pause(Reproductor *reproductor) override;
  std::string nombre() const override
  {
    return "Paused";
  }
};

// ---------------------------
// Reproductor
// ---------------------------
class Reproductor
{
public:
  VLC::MediaPlayer player;
  Estado *estado;

  Reproductor(VLC::Instance &instance);
  ~Reproductor();

  void setEstado(Estado *nuevo);
  void play();
  void pause();
  // new
  void enviarInfo();
  std::string getEstado() const;
};

// ---------------------------
// MusicServer (prototipo)
// ---------------------------
int MusicServer(int argc, char *argv[]);
