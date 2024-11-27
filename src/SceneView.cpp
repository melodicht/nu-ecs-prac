// #include "SceneView.hpp"

// SceneView::SceneView(Scene& scene) : pScene(&scene) 
// {
//   if (sizeof...(ComponentTypes) == 0)
//   {
//     all = true;
//   }
//   else
//   {
//     // Unpack the template parameters into an initializer list
//     int componentIds[] = { 0, GetId<ComponentTypes>() ... };
//     for (int i = 1; i < (sizeof...(ComponentTypes) + 1); i++)
//       componentMask.set(componentIds[i]);
//   }
// }


// SceneView::Iterator::Iterator(Scene* pScene, unsigned int index, ComponentMask mask, bool all) 
// : pScene(pScene), index(index), mask(mask), all(all) {}

// // give back the entityID we're currently at
// EntityID SceneView::Iterator::operator*() const 
// {
//   return pScene->entities[index].id; 
// }

//     // Compare two iterators
// bool SceneView::Iterator::operator==(const Iterator& other) const
// {
//   return index == other.index || index == pScene->entities.size();
// }
// bool SceneView::Iterator::operator!=(const Iterator& other) const
// {
//   return index != other.index && index != pScene->entities.size();
// }

// bool SceneView::Iterator::ValidIndex()
// {
//   return
//     // It's a valid entity ID
//     SceneView::IsEntityValid(pScene->entities[index].id) &&
//     // It has the correct component mask
//     (all || mask == (mask & pScene->entities[index].mask));
// }

// // Move the iterator forward
// SceneView::Iterator& SceneView::Iterator::operator++()
// {
//   do
//   {
//     index++;
//   } while (index < pScene->entities.size() && !ValidIndex());
//   return *this;
// }

// // Give an iterator to the beginning of this view
// const SceneView::Iterator SceneView::begin()
// {
//   int firstIndex = 0;
//   while (firstIndex < pScene->entities.size() &&
//     (componentMask != (componentMask & pScene->entities[firstIndex].mask) 
//       || !IsEntityValid(pScene->entities[firstIndex].id))) 
//   {
//     firstIndex++;
//   }
//   return Iterator(pScene, firstIndex, componentMask, all);
// }

// // Give an iterator to the end of this view 
// const SceneView::Iterator SceneView::end()
// {
//   return Iterator(pScene, (unsigned int)(pScene->entities.size()), componentMask, all);
// }