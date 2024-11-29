#ifndef SCENEVIEW_HPP
#define SCENEVIEW_HPP

#include "ECSconsts.hpp"
#include "Scene.hpp"

// Helps with iterating through a given scene
template<typename... ComponentTypes>
struct SceneView
{
  SceneView(Scene& scene);

  struct Iterator
  {
    Iterator(Scene* pScene, unsigned int index, ComponentMask mask, bool all);

    // give back the entityID we're currently at
    EntityID operator*() const;

    // Compare two iterators
    bool operator==(const Iterator& other) const;
    bool operator!=(const Iterator& other) const;

    bool ValidIndex();

    // Move the iterator forward
    Iterator& operator++();
    unsigned int index;
    Scene* pScene;
    ComponentMask mask;
    bool all{ false };
  };

  // Give an iterator to the beginning of this view
  const Iterator begin() const;

    // Give an iterator to the end of this view 
  const Iterator end() const;

  Scene* pScene{ nullptr };
  ComponentMask componentMask;
  bool all{ false };
};
#endif