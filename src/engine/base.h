#pragma once
#include <cstdint>
#include <vector>

// 1. Forward-declare Renderer so GlobalContext knows it's a class
class Engine;

// 2. Define Object and RenderObject FIRST so GlobalContext knows about
// RenderObject
class GlobalContext; // Forward declaration if needed, or place GlobalContext
                     // below

class Object {
public:
  Object(GlobalContext &context) : ctx(context) {};
  uint32_t get_object_uid();

protected:
  GlobalContext &ctx;
  uint32_t object_uid;

private:
};

class RenderObject : public Object {
public:
  RenderObject(GlobalContext &context);
  virtual bool draw();
};

// 3. Now GlobalContext can safely use RenderObject and Renderer*
class GlobalContext {
public:
  Engine *g_renderer;
  std::vector<RenderObject> drawCalls;
  uint32_t currentID = 0;
  void set_renderer(Engine *ren) { g_renderer = ren; }
};
