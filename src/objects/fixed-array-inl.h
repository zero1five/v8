// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef V8_OBJECTS_FIXED_ARRAY_INL_H_
#define V8_OBJECTS_FIXED_ARRAY_INL_H_

#include "src/objects/fixed-array.h"

#include "src/conversions.h"
#include "src/handles-inl.h"
#include "src/heap/heap-write-barrier.h"
#include "src/objects-inl.h"  // Needed for write barriers
#include "src/objects/bigint.h"
#include "src/objects/map.h"
#include "src/objects/maybe-object-inl.h"
#include "src/objects/slots.h"

// Has to be the last include (doesn't have include guards):
#include "src/objects/object-macros.h"

namespace v8 {
namespace internal {

OBJECT_CONSTRUCTORS_IMPL(FixedArrayBasePtr, HeapObjectPtr)
OBJECT_CONSTRUCTORS_IMPL(FixedArrayPtr, FixedArrayBasePtr)
OBJECT_CONSTRUCTORS_IMPL(FixedDoubleArray, FixedArrayBasePtr)
OBJECT_CONSTRUCTORS_IMPL(FixedTypedArrayBase, FixedArrayBasePtr)
OBJECT_CONSTRUCTORS_IMPL(ArrayList, FixedArrayPtr)
OBJECT_CONSTRUCTORS_IMPL(ByteArray, FixedArrayBasePtr)
OBJECT_CONSTRUCTORS_IMPL(TemplateList, FixedArrayPtr)

FixedArrayBasePtr::FixedArrayBasePtr(Address ptr,
                                     AllowInlineSmiStorage allow_smi)
    : HeapObjectPtr(ptr, allow_smi) {
  SLOW_DCHECK(
      (allow_smi == AllowInlineSmiStorage::kAllowBeingASmi && IsSmi()) ||
      IsFixedArrayBase());
}

ByteArray::ByteArray(Address ptr, AllowInlineSmiStorage allow_smi)
    : FixedArrayBasePtr(ptr, allow_smi) {
  SLOW_DCHECK(
      (allow_smi == AllowInlineSmiStorage::kAllowBeingASmi && IsSmi()) ||
      IsByteArray());
}

CAST_ACCESSOR2(ArrayList)
CAST_ACCESSOR2(ByteArray)
CAST_ACCESSOR(FixedArray)
CAST_ACCESSOR2(FixedArrayPtr)
CAST_ACCESSOR(FixedArrayBase)
CAST_ACCESSOR2(FixedArrayBasePtr)
CAST_ACCESSOR2(FixedDoubleArray)
CAST_ACCESSOR2(FixedTypedArrayBase)
CAST_ACCESSOR2(TemplateList)
CAST_ACCESSOR(WeakFixedArray)
CAST_ACCESSOR(WeakArrayList)

SMI_ACCESSORS(FixedArrayBase, length, kLengthOffset)
SMI_ACCESSORS(FixedArrayBasePtr, length, kLengthOffset)
SYNCHRONIZED_SMI_ACCESSORS(FixedArrayBase, length, kLengthOffset)
SYNCHRONIZED_SMI_ACCESSORS(FixedArrayBasePtr, length, kLengthOffset)
SMI_ACCESSORS(WeakFixedArray, length, kLengthOffset)
SYNCHRONIZED_SMI_ACCESSORS(WeakFixedArray, length, kLengthOffset)

SMI_ACCESSORS(WeakArrayList, capacity, kCapacityOffset)
SYNCHRONIZED_SMI_ACCESSORS(WeakArrayList, capacity, kCapacityOffset)
SMI_ACCESSORS(WeakArrayList, length, kLengthOffset)

Object* FixedArrayBase::unchecked_synchronized_length() const {
  return ACQUIRE_READ_FIELD(this, kLengthOffset);
}
Object* FixedArrayBasePtr::unchecked_synchronized_length() const {
  return ACQUIRE_READ_FIELD(this, kLengthOffset);
}

ACCESSORS(FixedTypedArrayBase, base_pointer, Object, kBasePointerOffset)

ObjectSlot FixedArray::GetFirstElementAddress() {
  return ObjectSlot(FIELD_ADDR(this, OffsetOfElementAt(0)));
}
ObjectSlot FixedArrayPtr::GetFirstElementAddress() {
  return ObjectSlot(FIELD_ADDR(this, OffsetOfElementAt(0)));
}

bool FixedArray::ContainsOnlySmisOrHoles() {
  Object* the_hole = GetReadOnlyRoots().the_hole_value();
  ObjectSlot current = GetFirstElementAddress();
  for (int i = 0; i < length(); ++i, ++current) {
    Object* candidate = *current;
    if (!candidate->IsSmi() && candidate != the_hole) return false;
  }
  return true;
}
bool FixedArrayPtr::ContainsOnlySmisOrHoles() {
  return reinterpret_cast<FixedArray*>(ptr())->ContainsOnlySmisOrHoles();
}

Object* FixedArray::get(int index) const {
  DCHECK(index >= 0 && index < this->length());
  return RELAXED_READ_FIELD(this, kHeaderSize + index * kTaggedSize);
}
Object* FixedArrayPtr::get(int index) const {
  DCHECK(index >= 0 && index < this->length());
  return RELAXED_READ_FIELD(this, kHeaderSize + index * kTaggedSize);
}

Handle<Object> FixedArray::get(FixedArray* array, int index, Isolate* isolate) {
  return handle(array->get(index), isolate);
}
Handle<Object> FixedArrayPtr::get(FixedArrayPtr array, int index,
                                  Isolate* isolate) {
  return handle(array->get(index), isolate);
}

template <class T>
MaybeHandle<T> FixedArray::GetValue(Isolate* isolate, int index) const {
  Object* obj = get(index);
  if (obj->IsUndefined(isolate)) return MaybeHandle<T>();
  return Handle<T>(T::cast(obj), isolate);
}
template <class T>
MaybeHandle<T> FixedArrayPtr::GetValue(Isolate* isolate, int index) const {
  Object* obj = get(index);
  if (obj->IsUndefined(isolate)) return MaybeHandle<T>();
  return Handle<T>(T::cast(obj), isolate);
}

template <class T>
Handle<T> FixedArray::GetValueChecked(Isolate* isolate, int index) const {
  Object* obj = get(index);
  CHECK(!obj->IsUndefined(isolate));
  return Handle<T>(T::cast(obj), isolate);
}
template <class T>
Handle<T> FixedArrayPtr::GetValueChecked(Isolate* isolate, int index) const {
  Object* obj = get(index);
  CHECK(!obj->IsUndefined(isolate));
  return Handle<T>(T::cast(obj), isolate);
}

bool FixedArray::is_the_hole(Isolate* isolate, int index) {
  return get(index)->IsTheHole(isolate);
}
bool FixedArrayPtr::is_the_hole(Isolate* isolate, int index) {
  return get(index)->IsTheHole(isolate);
}

void FixedArray::set(int index, Smi value) {
  DCHECK_NE(map(), GetReadOnlyRoots().fixed_cow_array_map());
  DCHECK_LT(index, this->length());
  DCHECK(ObjectPtr(value).IsSmi());
  int offset = kHeaderSize + index * kTaggedSize;
  RELAXED_WRITE_FIELD(this, offset, value);
}
void FixedArrayPtr::set(int index, Smi value) {
  reinterpret_cast<FixedArray*>(ptr())->set(index, value);
}

void FixedArray::set(int index, Object* value) {
  DCHECK_NE(GetReadOnlyRoots().fixed_cow_array_map(), map());
  DCHECK(IsFixedArray());
  DCHECK_GE(index, 0);
  DCHECK_LT(index, this->length());
  int offset = kHeaderSize + index * kTaggedSize;
  RELAXED_WRITE_FIELD(this, offset, value);
  WRITE_BARRIER(this, offset, value);
}
void FixedArrayPtr::set(int index, Object* value) {
  reinterpret_cast<FixedArray*>(ptr())->set(index, value);
}

void FixedArray::set(int index, Object* value, WriteBarrierMode mode) {
  DCHECK_NE(map(), GetReadOnlyRoots().fixed_cow_array_map());
  DCHECK_GE(index, 0);
  DCHECK_LT(index, this->length());
  int offset = kHeaderSize + index * kTaggedSize;
  RELAXED_WRITE_FIELD(this, offset, value);
  CONDITIONAL_WRITE_BARRIER(this, offset, value, mode);
}
void FixedArrayPtr::set(int index, Object* value, WriteBarrierMode mode) {
  reinterpret_cast<FixedArray*>(ptr())->set(index, value, mode);
}

void FixedArray::NoWriteBarrierSet(FixedArray* array, int index,
                                   Object* value) {
  DCHECK_NE(array->map(), array->GetReadOnlyRoots().fixed_cow_array_map());
  DCHECK_GE(index, 0);
  DCHECK_LT(index, array->length());
  DCHECK(!Heap::InNewSpace(value));
  RELAXED_WRITE_FIELD(array, kHeaderSize + index * kTaggedSize, value);
}
void FixedArrayPtr::NoWriteBarrierSet(FixedArrayPtr array, int index,
                                      Object* value) {
  DCHECK_NE(array->map(), array->GetReadOnlyRoots().fixed_cow_array_map());
  DCHECK_GE(index, 0);
  DCHECK_LT(index, array->length());
  DCHECK(!Heap::InNewSpace(value));
  RELAXED_WRITE_FIELD(array, kHeaderSize + index * kTaggedSize, value);
}

void FixedArray::set_undefined(int index) {
  set_undefined(GetReadOnlyRoots(), index);
}
void FixedArrayPtr::set_undefined(int index) {
  reinterpret_cast<FixedArray*>(ptr())->set_undefined(index);
}

void FixedArray::set_undefined(Isolate* isolate, int index) {
  set_undefined(ReadOnlyRoots(isolate), index);
}
void FixedArrayPtr::set_undefined(Isolate* isolate, int index) {
  reinterpret_cast<FixedArray*>(ptr())->set_undefined(isolate, index);
}

void FixedArray::set_undefined(ReadOnlyRoots ro_roots, int index) {
  FixedArray::NoWriteBarrierSet(this, index, ro_roots.undefined_value());
}

void FixedArray::set_null(int index) { set_null(GetReadOnlyRoots(), index); }
void FixedArrayPtr::set_null(int index) {
  reinterpret_cast<FixedArray*>(ptr())->set_null(index);
}

void FixedArray::set_null(Isolate* isolate, int index) {
  set_null(ReadOnlyRoots(isolate), index);
}
void FixedArrayPtr::set_null(Isolate* isolate, int index) {
  reinterpret_cast<FixedArray*>(ptr())->set_null(isolate, index);
}

void FixedArray::set_null(ReadOnlyRoots ro_roots, int index) {
  FixedArray::NoWriteBarrierSet(this, index, ro_roots.null_value());
}

void FixedArray::set_the_hole(int index) {
  set_the_hole(GetReadOnlyRoots(), index);
}
void FixedArrayPtr::set_the_hole(int index) {
  reinterpret_cast<FixedArray*>(ptr())->set_the_hole(index);
}

void FixedArray::set_the_hole(Isolate* isolate, int index) {
  set_the_hole(ReadOnlyRoots(isolate), index);
}
void FixedArrayPtr::set_the_hole(Isolate* isolate, int index) {
  reinterpret_cast<FixedArray*>(ptr())->set_the_hole(isolate, index);
}

void FixedArray::set_the_hole(ReadOnlyRoots ro_roots, int index) {
  FixedArray::NoWriteBarrierSet(this, index, ro_roots.the_hole_value());
}

void FixedArray::FillWithHoles(int from, int to) {
  for (int i = from; i < to; i++) {
    set_the_hole(i);
  }
}
void FixedArrayPtr::FillWithHoles(int from, int to) {
  for (int i = from; i < to; i++) {
    set_the_hole(i);
  }
}

ObjectSlot FixedArray::data_start() {
  return HeapObject::RawField(this, OffsetOfElementAt(0));
}
ObjectSlot FixedArrayPtr::data_start() {
  return RawField(OffsetOfElementAt(0));
}

ObjectSlot FixedArray::RawFieldOfElementAt(int index) {
  return HeapObject::RawField(this, OffsetOfElementAt(index));
}
ObjectSlot FixedArrayPtr::RawFieldOfElementAt(int index) {
  return RawField(OffsetOfElementAt(index));
}

void FixedArray::MoveElements(Heap* heap, int dst_index, int src_index, int len,
                              WriteBarrierMode mode) {
  DisallowHeapAllocation no_gc;
  heap->MoveElements(this, dst_index, src_index, len, mode);
}
void FixedArrayPtr::MoveElements(Heap* heap, int dst_index, int src_index,
                                 int len, WriteBarrierMode mode) {
  DisallowHeapAllocation no_gc;
  heap->MoveElements(reinterpret_cast<FixedArray*>(ptr()), dst_index, src_index,
                     len, mode);
}

double FixedDoubleArray::get_scalar(int index) {
  DCHECK(map() != GetReadOnlyRoots().fixed_cow_array_map() &&
         map() != GetReadOnlyRoots().fixed_array_map());
  DCHECK(index >= 0 && index < this->length());
  DCHECK(!is_the_hole(index));
  return READ_DOUBLE_FIELD(this, kHeaderSize + index * kDoubleSize);
}

uint64_t FixedDoubleArray::get_representation(int index) {
  DCHECK(map() != GetReadOnlyRoots().fixed_cow_array_map() &&
         map() != GetReadOnlyRoots().fixed_array_map());
  DCHECK(index >= 0 && index < this->length());
  int offset = kHeaderSize + index * kDoubleSize;
  return READ_UINT64_FIELD(this, offset);
}

Handle<Object> FixedDoubleArray::get(FixedDoubleArray array, int index,
                                     Isolate* isolate) {
  if (array->is_the_hole(index)) {
    return isolate->factory()->the_hole_value();
  } else {
    return isolate->factory()->NewNumber(array->get_scalar(index));
  }
}

void FixedDoubleArray::set(int index, double value) {
  DCHECK(map() != GetReadOnlyRoots().fixed_cow_array_map() &&
         map() != GetReadOnlyRoots().fixed_array_map());
  int offset = kHeaderSize + index * kDoubleSize;
  if (std::isnan(value)) {
    WRITE_DOUBLE_FIELD(this, offset, std::numeric_limits<double>::quiet_NaN());
  } else {
    WRITE_DOUBLE_FIELD(this, offset, value);
  }
  DCHECK(!is_the_hole(index));
}

void FixedDoubleArray::set_the_hole(Isolate* isolate, int index) {
  set_the_hole(index);
}

void FixedDoubleArray::set_the_hole(int index) {
  DCHECK(map() != GetReadOnlyRoots().fixed_cow_array_map() &&
         map() != GetReadOnlyRoots().fixed_array_map());
  int offset = kHeaderSize + index * kDoubleSize;
  WRITE_UINT64_FIELD(this, offset, kHoleNanInt64);
}

bool FixedDoubleArray::is_the_hole(Isolate* isolate, int index) {
  return is_the_hole(index);
}

bool FixedDoubleArray::is_the_hole(int index) {
  return get_representation(index) == kHoleNanInt64;
}

void FixedDoubleArray::MoveElements(Heap* heap, int dst_index, int src_index,
                                    int len, WriteBarrierMode mode) {
  DCHECK_EQ(SKIP_WRITE_BARRIER, mode);
  double* data_start = reinterpret_cast<double*>(FIELD_ADDR(this, kHeaderSize));
  MemMove(data_start + dst_index, data_start + src_index, len * kDoubleSize);
}

void FixedDoubleArray::FillWithHoles(int from, int to) {
  for (int i = from; i < to; i++) {
    set_the_hole(i);
  }
}

MaybeObject WeakFixedArray::Get(int index) const {
  DCHECK(index >= 0 && index < this->length());
  return RELAXED_READ_WEAK_FIELD(this, OffsetOfElementAt(index));
}

void WeakFixedArray::Set(int index, MaybeObject value) {
  DCHECK_GE(index, 0);
  DCHECK_LT(index, length());
  int offset = OffsetOfElementAt(index);
  RELAXED_WRITE_FIELD(this, offset, value);
  WEAK_WRITE_BARRIER(this, offset, value);
}

void WeakFixedArray::Set(int index, MaybeObject value, WriteBarrierMode mode) {
  DCHECK_GE(index, 0);
  DCHECK_LT(index, length());
  int offset = OffsetOfElementAt(index);
  RELAXED_WRITE_FIELD(this, offset, value);
  CONDITIONAL_WEAK_WRITE_BARRIER(this, offset, value, mode);
}

MaybeObjectSlot WeakFixedArray::data_start() {
  return HeapObject::RawMaybeWeakField(this, kHeaderSize);
}

MaybeObjectSlot WeakFixedArray::RawFieldOfElementAt(int index) {
  return HeapObject::RawMaybeWeakField(this, OffsetOfElementAt(index));
}

MaybeObject WeakArrayList::Get(int index) const {
  DCHECK(index >= 0 && index < this->capacity());
  return RELAXED_READ_WEAK_FIELD(this, OffsetOfElementAt(index));
}

void WeakArrayList::Set(int index, MaybeObject value, WriteBarrierMode mode) {
  DCHECK_GE(index, 0);
  DCHECK_LT(index, this->capacity());
  int offset = OffsetOfElementAt(index);
  RELAXED_WRITE_FIELD(this, offset, value);
  CONDITIONAL_WEAK_WRITE_BARRIER(this, offset, value, mode);
}

MaybeObjectSlot WeakArrayList::data_start() {
  return HeapObject::RawMaybeWeakField(this, kHeaderSize);
}

HeapObject* WeakArrayList::Iterator::Next() {
  if (array_ != nullptr) {
    while (index_ < array_->length()) {
      MaybeObject item = array_->Get(index_++);
      DCHECK(item->IsWeakOrCleared());
      if (!item->IsCleared()) return item->GetHeapObjectAssumeWeak();
    }
    array_ = nullptr;
  }
  return nullptr;
}

int ArrayList::Length() const {
  if (FixedArray::cast(*this)->length() == 0) return 0;
  return Smi::ToInt(FixedArray::cast(*this)->get(kLengthIndex));
}

void ArrayList::SetLength(int length) {
  return FixedArray::cast(*this)->set(kLengthIndex, Smi::FromInt(length));
}

Object* ArrayList::Get(int index) const {
  return FixedArray::cast(*this)->get(kFirstIndex + index);
}

ObjectSlot ArrayList::Slot(int index) {
  return RawField(OffsetOfElementAt(kFirstIndex + index));
}

void ArrayList::Set(int index, Object* obj, WriteBarrierMode mode) {
  FixedArray::cast(*this)->set(kFirstIndex + index, obj, mode);
}

void ArrayList::Clear(int index, Object* undefined) {
  DCHECK(undefined->IsUndefined());
  FixedArray::cast(*this)->set(kFirstIndex + index, undefined,
                               SKIP_WRITE_BARRIER);
}

int ByteArray::Size() { return RoundUp(length() + kHeaderSize, kTaggedSize); }

byte ByteArray::get(int index) const {
  DCHECK(index >= 0 && index < this->length());
  return READ_BYTE_FIELD(this, kHeaderSize + index * kCharSize);
}

void ByteArray::set(int index, byte value) {
  DCHECK(index >= 0 && index < this->length());
  WRITE_BYTE_FIELD(this, kHeaderSize + index * kCharSize, value);
}

void ByteArray::copy_in(int index, const byte* buffer, int length) {
  DCHECK(index >= 0 && length >= 0 && length <= kMaxInt - index &&
         index + length <= this->length());
  Address dst_addr = FIELD_ADDR(this, kHeaderSize + index * kCharSize);
  memcpy(reinterpret_cast<void*>(dst_addr), buffer, length);
}

void ByteArray::copy_out(int index, byte* buffer, int length) {
  DCHECK(index >= 0 && length >= 0 && length <= kMaxInt - index &&
         index + length <= this->length());
  Address src_addr = FIELD_ADDR(this, kHeaderSize + index * kCharSize);
  memcpy(buffer, reinterpret_cast<void*>(src_addr), length);
}

int ByteArray::get_int(int index) const {
  DCHECK(index >= 0 && index < this->length() / kIntSize);
  return READ_INT_FIELD(this, kHeaderSize + index * kIntSize);
}

void ByteArray::set_int(int index, int value) {
  DCHECK(index >= 0 && index < this->length() / kIntSize);
  WRITE_INT_FIELD(this, kHeaderSize + index * kIntSize, value);
}

uint32_t ByteArray::get_uint32(int index) const {
  DCHECK(index >= 0 && index < this->length() / kUInt32Size);
  return READ_UINT32_FIELD(this, kHeaderSize + index * kUInt32Size);
}

void ByteArray::set_uint32(int index, uint32_t value) {
  DCHECK(index >= 0 && index < this->length() / kUInt32Size);
  WRITE_UINT32_FIELD(this, kHeaderSize + index * kUInt32Size, value);
}

void ByteArray::clear_padding() {
  int data_size = length() + kHeaderSize;
  memset(reinterpret_cast<void*>(address() + data_size), 0, Size() - data_size);
}

ByteArray ByteArray::FromDataStartAddress(Address address) {
  DCHECK_TAG_ALIGNED(address);
  return ByteArray::cast(ObjectPtr(address - kHeaderSize + kHeapObjectTag));
}

int ByteArray::DataSize() const { return RoundUp(length(), kTaggedSize); }

int ByteArray::ByteArraySize() { return SizeFor(this->length()); }

byte* ByteArray::GetDataStartAddress() {
  return reinterpret_cast<byte*>(address() + kHeaderSize);
}

byte* ByteArray::GetDataEndAddress() {
  return GetDataStartAddress() + length();
}

template <class T>
PodArray<T>::PodArray(Address ptr) : ByteArray(ptr) {}

template <class T>
PodArray<T> PodArray<T>::cast(Object* object) {
  return PodArray<T>(object->ptr());
}

template <class T>
PodArray<T> PodArray<T>::cast(ObjectPtr object) {
  return PodArray<T>(object.ptr());
}

// static
template <class T>
Handle<PodArray<T>> PodArray<T>::New(Isolate* isolate, int length,
                                     PretenureFlag pretenure) {
  return Handle<PodArray<T>>::cast(
      isolate->factory()->NewByteArray(length * sizeof(T), pretenure));
}

template <class T>
int PodArray<T>::length() const {
  return ByteArray::length() / sizeof(T);
}

void* FixedTypedArrayBase::external_pointer() const {
  intptr_t ptr = READ_INTPTR_FIELD(this, kExternalPointerOffset);
  return reinterpret_cast<void*>(ptr);
}

void FixedTypedArrayBase::set_external_pointer(void* value,
                                               WriteBarrierMode mode) {
  intptr_t ptr = reinterpret_cast<intptr_t>(value);
  WRITE_INTPTR_FIELD(this, kExternalPointerOffset, ptr);
}

void* FixedTypedArrayBase::DataPtr() {
  return reinterpret_cast<void*>(
      reinterpret_cast<intptr_t>(base_pointer()) +
      reinterpret_cast<intptr_t>(external_pointer()));
}

int FixedTypedArrayBase::ElementSize(InstanceType type) {
  int element_size;
  switch (type) {
#define TYPED_ARRAY_CASE(Type, type, TYPE, ctype) \
  case FIXED_##TYPE##_ARRAY_TYPE:                 \
    element_size = sizeof(ctype);                 \
    break;

    TYPED_ARRAYS(TYPED_ARRAY_CASE)
#undef TYPED_ARRAY_CASE
    default:
      UNREACHABLE();
  }
  return element_size;
}

int FixedTypedArrayBase::DataSize(InstanceType type) const {
  if (base_pointer() == Smi::kZero) return 0;
  return length() * ElementSize(type);
}

int FixedTypedArrayBase::DataSize() const {
  return DataSize(map()->instance_type());
}

size_t FixedTypedArrayBase::ByteLength() const {
  return static_cast<size_t>(length()) *
         static_cast<size_t>(ElementSize(map()->instance_type()));
}

int FixedTypedArrayBase::size() const {
  return OBJECT_POINTER_ALIGN(kDataOffset + DataSize());
}

int FixedTypedArrayBase::TypedArraySize(InstanceType type) const {
  return OBJECT_POINTER_ALIGN(kDataOffset + DataSize(type));
}

// static
int FixedTypedArrayBase::TypedArraySize(InstanceType type, int length) {
  return OBJECT_POINTER_ALIGN(kDataOffset + length * ElementSize(type));
}

uint8_t Uint8ArrayTraits::defaultValue() { return 0; }

uint8_t Uint8ClampedArrayTraits::defaultValue() { return 0; }

int8_t Int8ArrayTraits::defaultValue() { return 0; }

uint16_t Uint16ArrayTraits::defaultValue() { return 0; }

int16_t Int16ArrayTraits::defaultValue() { return 0; }

uint32_t Uint32ArrayTraits::defaultValue() { return 0; }

int32_t Int32ArrayTraits::defaultValue() { return 0; }

float Float32ArrayTraits::defaultValue() {
  return std::numeric_limits<float>::quiet_NaN();
}

double Float64ArrayTraits::defaultValue() {
  return std::numeric_limits<double>::quiet_NaN();
}

template <class Traits>
typename Traits::ElementType FixedTypedArray<Traits>::get_scalar(int index) {
  DCHECK((index >= 0) && (index < this->length()));
  return FixedTypedArray<Traits>::get_scalar_from_data_ptr(DataPtr(), index);
}

// static
template <class Traits>
typename Traits::ElementType FixedTypedArray<Traits>::get_scalar_from_data_ptr(
    void* data_ptr, int index) {
  typename Traits::ElementType* ptr = reinterpret_cast<ElementType*>(data_ptr);
  // The JavaScript memory model allows for racy reads and writes to a
  // SharedArrayBuffer's backing store, which will always be a FixedTypedArray.
  // ThreadSanitizer will catch these racy accesses and warn about them, so we
  // disable TSAN for these reads and writes using annotations.
  //
  // We don't use relaxed atomics here, as it is not a requirement of the
  // JavaScript memory model to have tear-free reads of overlapping accesses,
  // and using relaxed atomics may introduce overhead.
  TSAN_ANNOTATE_IGNORE_READS_BEGIN;
  auto result = ptr[index];
  TSAN_ANNOTATE_IGNORE_READS_END;
  return result;
}

template <class Traits>
void FixedTypedArray<Traits>::set(int index, ElementType value) {
  CHECK((index >= 0) && (index < this->length()));
  // See the comment in FixedTypedArray<Traits>::get_scalar.
  auto* ptr = reinterpret_cast<ElementType*>(DataPtr());
  TSAN_ANNOTATE_IGNORE_WRITES_BEGIN;
  ptr[index] = value;
  TSAN_ANNOTATE_IGNORE_WRITES_END;
}

template <class Traits>
typename Traits::ElementType FixedTypedArray<Traits>::from(int value) {
  return static_cast<ElementType>(value);
}

template <>
inline uint8_t FixedTypedArray<Uint8ClampedArrayTraits>::from(int value) {
  if (value < 0) return 0;
  if (value > 0xFF) return 0xFF;
  return static_cast<uint8_t>(value);
}

template <>
inline int64_t FixedTypedArray<BigInt64ArrayTraits>::from(int value) {
  UNREACHABLE();
}

template <>
inline uint64_t FixedTypedArray<BigUint64ArrayTraits>::from(int value) {
  UNREACHABLE();
}

template <class Traits>
typename Traits::ElementType FixedTypedArray<Traits>::from(uint32_t value) {
  return static_cast<ElementType>(value);
}

template <>
inline uint8_t FixedTypedArray<Uint8ClampedArrayTraits>::from(uint32_t value) {
  // We need this special case for Uint32 -> Uint8Clamped, because the highest
  // Uint32 values will be negative as an int, clamping to 0, rather than 255.
  if (value > 0xFF) return 0xFF;
  return static_cast<uint8_t>(value);
}

template <>
inline int64_t FixedTypedArray<BigInt64ArrayTraits>::from(uint32_t value) {
  UNREACHABLE();
}

template <>
inline uint64_t FixedTypedArray<BigUint64ArrayTraits>::from(uint32_t value) {
  UNREACHABLE();
}

template <class Traits>
typename Traits::ElementType FixedTypedArray<Traits>::from(double value) {
  return static_cast<ElementType>(DoubleToInt32(value));
}

template <>
inline uint8_t FixedTypedArray<Uint8ClampedArrayTraits>::from(double value) {
  // Handle NaNs and less than zero values which clamp to zero.
  if (!(value > 0)) return 0;
  if (value > 0xFF) return 0xFF;
  return static_cast<uint8_t>(lrint(value));
}

template <>
inline int64_t FixedTypedArray<BigInt64ArrayTraits>::from(double value) {
  UNREACHABLE();
}

template <>
inline uint64_t FixedTypedArray<BigUint64ArrayTraits>::from(double value) {
  UNREACHABLE();
}

template <>
inline float FixedTypedArray<Float32ArrayTraits>::from(double value) {
  return static_cast<float>(value);
}

template <>
inline double FixedTypedArray<Float64ArrayTraits>::from(double value) {
  return value;
}

template <class Traits>
typename Traits::ElementType FixedTypedArray<Traits>::from(int64_t value) {
  UNREACHABLE();
}

template <class Traits>
typename Traits::ElementType FixedTypedArray<Traits>::from(uint64_t value) {
  UNREACHABLE();
}

template <>
inline int64_t FixedTypedArray<BigInt64ArrayTraits>::from(int64_t value) {
  return value;
}

template <>
inline uint64_t FixedTypedArray<BigUint64ArrayTraits>::from(uint64_t value) {
  return value;
}

template <>
inline uint64_t FixedTypedArray<BigUint64ArrayTraits>::from(int64_t value) {
  return static_cast<uint64_t>(value);
}

template <>
inline int64_t FixedTypedArray<BigInt64ArrayTraits>::from(uint64_t value) {
  return static_cast<int64_t>(value);
}

template <class Traits>
typename Traits::ElementType FixedTypedArray<Traits>::FromHandle(
    Handle<Object> value, bool* lossless) {
  if (value->IsSmi()) {
    return from(Smi::ToInt(*value));
  }
  DCHECK(value->IsHeapNumber());
  return from(HeapNumber::cast(*value)->value());
}

template <>
inline int64_t FixedTypedArray<BigInt64ArrayTraits>::FromHandle(
    Handle<Object> value, bool* lossless) {
  DCHECK(value->IsBigInt());
  return BigInt::cast(*value)->AsInt64(lossless);
}

template <>
inline uint64_t FixedTypedArray<BigUint64ArrayTraits>::FromHandle(
    Handle<Object> value, bool* lossless) {
  DCHECK(value->IsBigInt());
  return BigInt::cast(*value)->AsUint64(lossless);
}

template <class Traits>
Handle<Object> FixedTypedArray<Traits>::get(Isolate* isolate,
                                            FixedTypedArray<Traits> array,
                                            int index) {
  return Traits::ToHandle(isolate, array->get_scalar(index));
}

template <class Traits>
void FixedTypedArray<Traits>::SetValue(uint32_t index, Object* value) {
  ElementType cast_value = Traits::defaultValue();
  if (value->IsSmi()) {
    int int_value = Smi::ToInt(value);
    cast_value = from(int_value);
  } else if (value->IsHeapNumber()) {
    double double_value = HeapNumber::cast(value)->value();
    cast_value = from(double_value);
  } else {
    // Clamp undefined to the default value. All other types have been
    // converted to a number type further up in the call chain.
    DCHECK(value->IsUndefined());
  }
  set(index, cast_value);
}

template <>
inline void FixedTypedArray<BigInt64ArrayTraits>::SetValue(uint32_t index,
                                                           Object* value) {
  DCHECK(value->IsBigInt());
  set(index, BigInt::cast(value)->AsInt64());
}

template <>
inline void FixedTypedArray<BigUint64ArrayTraits>::SetValue(uint32_t index,
                                                            Object* value) {
  DCHECK(value->IsBigInt());
  set(index, BigInt::cast(value)->AsUint64());
}

Handle<Object> Uint8ArrayTraits::ToHandle(Isolate* isolate, uint8_t scalar) {
  return handle(Smi::FromInt(scalar), isolate);
}

Handle<Object> Uint8ClampedArrayTraits::ToHandle(Isolate* isolate,
                                                 uint8_t scalar) {
  return handle(Smi::FromInt(scalar), isolate);
}

Handle<Object> Int8ArrayTraits::ToHandle(Isolate* isolate, int8_t scalar) {
  return handle(Smi::FromInt(scalar), isolate);
}

Handle<Object> Uint16ArrayTraits::ToHandle(Isolate* isolate, uint16_t scalar) {
  return handle(Smi::FromInt(scalar), isolate);
}

Handle<Object> Int16ArrayTraits::ToHandle(Isolate* isolate, int16_t scalar) {
  return handle(Smi::FromInt(scalar), isolate);
}

Handle<Object> Uint32ArrayTraits::ToHandle(Isolate* isolate, uint32_t scalar) {
  return isolate->factory()->NewNumberFromUint(scalar);
}

Handle<Object> Int32ArrayTraits::ToHandle(Isolate* isolate, int32_t scalar) {
  return isolate->factory()->NewNumberFromInt(scalar);
}

Handle<Object> Float32ArrayTraits::ToHandle(Isolate* isolate, float scalar) {
  return isolate->factory()->NewNumber(scalar);
}

Handle<Object> Float64ArrayTraits::ToHandle(Isolate* isolate, double scalar) {
  return isolate->factory()->NewNumber(scalar);
}

Handle<Object> BigInt64ArrayTraits::ToHandle(Isolate* isolate, int64_t scalar) {
  return BigInt::FromInt64(isolate, scalar);
}

Handle<Object> BigUint64ArrayTraits::ToHandle(Isolate* isolate,
                                              uint64_t scalar) {
  return BigInt::FromUint64(isolate, scalar);
}

// static
template <class Traits>
STATIC_CONST_MEMBER_DEFINITION const InstanceType
    FixedTypedArray<Traits>::kInstanceType;

template <class Traits>
FixedTypedArray<Traits>::FixedTypedArray(Address ptr)
    : FixedTypedArrayBase(ptr) {
  DCHECK(IsHeapObject() && map()->instance_type() == Traits::kInstanceType);
}

template <class Traits>
FixedTypedArray<Traits> FixedTypedArray<Traits>::cast(Object* object) {
  return FixedTypedArray<Traits>(object->ptr());
}

template <class Traits>
FixedTypedArray<Traits> FixedTypedArray<Traits>::cast(ObjectPtr object) {
  return FixedTypedArray<Traits>(object.ptr());
}

int TemplateList::length() const {
  return Smi::ToInt(FixedArray::cast(*this)->get(kLengthIndex));
}

Object* TemplateList::get(int index) const {
  return FixedArray::cast(*this)->get(kFirstElementIndex + index);
}

void TemplateList::set(int index, Object* value) {
  FixedArray::cast(*this)->set(kFirstElementIndex + index, value);
}

}  // namespace internal
}  // namespace v8

#include "src/objects/object-macros-undef.h"

#endif  // V8_OBJECTS_FIXED_ARRAY_INL_H_
