#ifndef SCENEVIEW_HPP
#define SCENEVIEW_HPP

#include "ECSConsts.hpp"
#include "Scene.hpp"

// Helps with iterating through a given scene
template<typename... ComponentTypes>
struct SceneView
{
  SceneView(Scene& scene)  : pScene(&scene) 
  {
    if (sizeof...(ComponentTypes) == 0)
    {
      all = true;
    }
    else
    {
      // Unpack the template parameters into an initializer list
      int componentIds[] = { 0, ECSConsts::GetId<ComponentTypes>() ... };
      for (int i = 1; i < (sizeof...(ComponentTypes) + 1); i++)
        componentMask.set(componentIds[i]);
    }
  }

  struct Iterator
  {
    Iterator(Scene* pScene, unsigned int index, ComponentMask mask, bool all) 
    : pScene(pScene), index(index), mask(mask), all(all) {}

    // give back the entityID we're currently at
    EntityID operator*() const{
      return pScene->entities[index].id; 
    }

    // Compare two iterators
    bool operator==(const Iterator& other) const{
      return index == other.index || index == pScene->entities.size();
    }
    bool operator!=(const Iterator& other) const{
      return index != other.index && index != pScene->entities.size();
    }

    bool ValidIndex(){
      return
      // It's a valid entity ID
      ECSConsts::IsEntityValid(pScene->entities[index].id) &&
      // It has the correct component mask
      (all || mask == (mask & pScene->entities[index].mask));
    }

    // Move the iterator forward
    Iterator& operator++(){
      do
      {
        index++;
      } while (index < pScene->entities.size() && !ValidIndex());
      return *this;
    }
    unsigned int index;
    Scene* pScene;
    ComponentMask mask;
    bool all{ false };
  };

  // Give an iterator to the beginning of this view
  const Iterator begin() const{
    int firstIndex = 0;
    while (firstIndex < pScene->entities.size() &&
      (componentMask != (componentMask & pScene->entities[firstIndex].mask) 
        || !ECSConsts::IsEntityValid(pScene->entities[firstIndex].id))) 
    {
      firstIndex++;
    }
    return Iterator(pScene, firstIndex, componentMask, all);
  }

  // Give an iterator to the end of this view 
  const Iterator end() const{
    return Iterator(pScene, (unsigned int)(pScene->entities.size()), componentMask, all);
  }

  Scene* pScene{ nullptr };
  ComponentMask componentMask;
  bool all{ false };
};

#endif