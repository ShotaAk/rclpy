// Copyright 2020 Open Source Robotics Foundation, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef RCLPY_COMMON__HANDLE_H_
#define RCLPY_COMMON__HANDLE_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <Python.h>
#include <stddef.h>

#include "rcutils/types/rcutils_ret.h"

/// Wrapper that manages the lifetime of an object,
/// allowing to establish dependencies between them.
typedef struct rclpy_handle_t rclpy_handle_t;

/// Signature of rclpy_handle_t destructors.
typedef void (* rclpy_handle_destructor_t)(void *);

/// Create a PyCapsule wrapping a rclpy_handle_t object.
/**
 * Handles are used to manage lifetime of rcl objects.
 * The main reason to use them, is that destruction order can't be controlled in Python.
 * From PEP 442:
 * > Cyclic isolate (CI)
 * > A standalone subgraph of objects in which no object is referenced from the outside,
 * > containing one or several reference cycles, and whose objects are still in a usable,
 * > non-broken state: they can access each other from their respective finalizers.
 * >
 * > For CI objects, the order in which finalizers are called (step 2 above) is undefined.
 *
 * Handles provide a basic way of establishing dependencies and keeping the reference count,
 * with a mechanism completly decoupled of Python reference counting.
 *
 * If in the future this library is migrated to use `pybind11`, this class can be dropped and
 * `std::shared_ptr` be used directly.
 *
 * \sa _rclpy_create_handle
 * \param name Name of the PyCapsule.
 * \returns PyCapsule wrapping the rclpy_handle_t.
 */
PyObject *
rclpy_create_handle_capsule(void * ptr, const char * name, rclpy_handle_destructor_t destructor);

/// Returns the object managed by the rclpy_handle_t wrapped in a PyCapsule.
void *
rclpy_handle_get_pointer_from_capsule(PyObject * capsule, const char * name);

/// Creates a rclpy_handle_t object.
/**
 * \param ptr Opaque pointer to the object being wrapped.
 * \param destructor Function that will be called when the handle is destructed.
 */
rclpy_handle_t *
_rclpy_create_handle(void * ptr, rclpy_handle_destructor_t destructor);

/// Create a PyCapsule wrapping a rclpy_handle_t object.
/**
 * \param ptr Already constructed handle.
 * \param name Name of the PyCapsule.
 * \returns PyCapsule wrapping the rclpy_handle_t, using _rclpy_handle_dec_ref as destructor.
 */
PyObject *
_rclpy_create_handle_capsule(rclpy_handle_t * ptr, const char * name);

/// Returns the object managed by the rclpy_handle_t.
/**
 * PyExc_RuntimeError is set, if `handle` is NULL, o if the managed pointer is NULL.
 *
 * \param handle
 */
void *
_rclpy_handle_get_pointer(rclpy_handle_t * handle);

/// Adds a dependency to a handle.
/**
 * PyExc_RuntimeError is set, if `handle` is NULL, o if the managed pointer is NULL.
 *
 * \param dependency Handle object whose reference count will be incremented.
 * \param dependent Handle object that keeps a reference to the `dependency`.
 */
void
_rclpy_handle_add_dependency(rclpy_handle_t * dependent, rclpy_handle_t * dependency);

/// Decrements the reference count of a handle.
/**
 * The reference count of `handle` is decremented.
 * If it reaches zero:
 * - `rclpy_handle_dec_ref` is called on `handle` dependencies.
 * - `handle` is deallocated.
 *
 * \param handle Object which reference count will be decremented.
 */
void
_rclpy_handle_dec_ref(rclpy_handle_t * handle);

#ifdef __cplusplus
}
#endif

#endif  // RCLPY_COMMON__HANDLE_H_
