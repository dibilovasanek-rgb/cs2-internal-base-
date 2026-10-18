#pragma once
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <unordered_map>
namespace WorldColors {
struct RGB { std::uint8_t r,g,b; };
struct Saved { RGB original, applied; };
class Cache {
 std::mutex mutex;
 std::byte* data = nullptr;
 std::unordered_map<std::size_t,Saved> saved;
 static RGB Read(std::byte* p) {auto* c=reinterpret_cast<std::uint8_t*>(p);return {c[0],c[1],c[2]};}
 static bool Equal(RGB a, RGB b) {return a.r==b.r && a.g==b.g && a.b==b.b;}
 void Update(std::size_t index, bool enabled, RGB color) {
  auto* p=data+index*0x20;const RGB current=Read(p);
  auto [it,added]=saved.try_emplace(index,Saved{current,current});auto& entry=it->second;
  if (!added && !Equal(current,entry.applied)) entry.original=current;
  const RGB result=enabled?color:entry.original;
  auto* bytes=reinterpret_cast<std::uint8_t*>(p);bytes[0]=result.r;bytes[1]=result.g;bytes[2]=result.b;
  entry.applied=result;
 }
 bool Bind(std::byte* buffer,std::size_t capacity) {
  if(buffer!=data){saved.clear();data=buffer;}
  return data && capacity<= (1u<<22);
 }
public:
 void Range(std::byte* buffer,std::size_t capacity,std::size_t first,std::size_t count,bool enabled,RGB color) {
  std::lock_guard lock(mutex);
  if(!Bind(buffer,capacity) || first>capacity || count>capacity-first || count>(1u<<20)) return;
  for(std::size_t i=first;i<first+count;++i) Update(i,enabled,color);
 }
 void Refresh(std::byte* buffer,std::size_t capacity,bool enabled,RGB color) {
  std::lock_guard lock(mutex);if(!Bind(buffer,capacity)) return;
  for(auto& item:saved) if(item.first<capacity) Update(item.first,enabled,color);
 }
 void Reset(){std::lock_guard lock(mutex);saved.clear();data=nullptr;}
};
}
