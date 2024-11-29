#include "SceneView.hpp"

template<typename... ComponentTypes> 
SceneView<ComponentTypes...>::SceneView(Scene& scene) : pScene(&scene) 
{
  if (sizeof...(ComponentTypes) == 0)
  {
    all = true;
  }
  else
  {
    // Unpack the template parameters into an initializer list
    int componentIds[] = { 0, ECSconsts::GetId<ComponentTypes>() ... };
    for (int i = 1; i < (sizeof...(ComponentTypes) + 1); i++)
      componentMask.set(componentIds[i]);
  }
}

template<typename... ComponentTypes> 
SceneView<ComponentTypes...>::Iterator::Iterator(Scene* pScene, unsigned int index, ComponentMask mask, bool all) 
: pScene(pScene), index(index), mask(mask), all(all) {}

template<typename... ComponentTypes>
// give back the entityID we're currently at
EntityID SceneView<ComponentTypes...>::Iterator::operator*() const 
{
  return pScene->entities[index].id; 
}

template<typename... ComponentTypes>
// Compare two iterators
bool SceneView<ComponentTypes...>::Iterator::operator==(const Iterator& other) const
{
  return index == other.index || index == pScene->entities.size();
}
template<typename... ComponentTypes>
bool SceneView<ComponentTypes...>::Iterator::operator!=(const Iterator& other) const
{
  return index != other.index && index != pScene->entities.size();
}

template<typename... ComponentTypes>
bool SceneView<ComponentTypes...>::Iterator::ValidIndex()
{
  return
    // It's a valid entity ID
    ECSconsts::IsEntityValid(pScene->entities[index].id) &&
    // It has the correct component mask
    (all || mask == (mask & pScene->entities[index].mask));
}

template<typename... ComponentTypes>
// Move the iterator forward
typename SceneView<ComponentTypes...>::Iterator& SceneView<ComponentTypes...>::Iterator::operator++()
{
  do
  {
    index++;
  } while (index < pScene->entities.size() && !ValidIndex());
  return *this;
}

template<typename... ComponentTypes>
// Give an iterator to the beginning of this view
const typename SceneView<ComponentTypes...>::Iterator SceneView<ComponentTypes...>::begin() const
{
  int firstIndex = 0;
  while (firstIndex < pScene->entities.size() &&
    (componentMask != (componentMask & pScene->entities[firstIndex].mask) 
      || !ECSconsts::IsEntityValid(pScene->entities[firstIndex].id))) 
  {
    firstIndex++;
  }
  return Iterator(pScene, firstIndex, componentMask, all);
}

template<typename... ComponentTypes>
// Give an iterator to the end of this view 
const typename SceneView<ComponentTypes...>::Iterator SceneView<ComponentTypes...>::end() const
{
  return Iterator(pScene, (unsigned int)(pScene->entities.size()), componentMask, all);
}