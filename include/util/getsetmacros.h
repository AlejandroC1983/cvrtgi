/*
Copyright 2022 Alejandro Cosin & Gustavo Patow

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

// Taken from https://gist.github.com/aminzai/2706798

#ifndef _GETSETMACROS_H_
#define _GETSETMACROS_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES

// CLASS FORWARDING

// NAMESPACE

// DEFINES
#define GET(Type, MemberName, FaceName) \
          const Type &get##FaceName() const { return MemberName; }

#define GETCOPY(Type, MemberName, FaceName) \
          const Type get##FaceName() const { return MemberName; }

#define REF(Type, MemberName, FaceName) \
          Type &ref##FaceName() { return MemberName; }

#define REFCOPY(Type, MemberName, FaceName) \
          Type ref##FaceName() { return MemberName; }

#define REF_PTR(Type, MemberName, FaceName) \
          Type *ref##FaceName() { return MemberName; }

#define GET_RETURN_PTR(Type, MemberName, FaceName) \
          const Type *get##FaceName() const { return &MemberName; }

#define REF_RETURN_PTR(Type, MemberName, FaceName) \
          Type *ref##FaceName() { return &MemberName; }

#define GET_PTR(Type, MemberName, FaceName) \
          const Type *get##FaceName() const { return MemberName; }

#define SET(Type, MemberName, FaceName) \
          void set##FaceName(const Type &value) { MemberName = value; }

#define SET_PTR(Type, MemberName, FaceName) \
          void set##FaceName(Type *value) { MemberName = value; }

#define REF_SET(Type, MemberName, FaceName) \
          Type &ref##FaceName() { return MemberName; }; \
          void  set##FaceName(const Type &value) { MemberName = value; }

#define REFCOPY_SET(Type, MemberName, FaceName) \
          Type ref##FaceName() { return MemberName; }; \
          void set##FaceName(const Type &value) { MemberName = value; }

#define GET_SET(Type, MemberName, FaceName) \
          const Type &get##FaceName() const { return MemberName; }; \
          void        set##FaceName(const Type &value) { MemberName = value; }

#define GETCOPY_SET(Type, MemberName, FaceName) \
          const Type get##FaceName() const { return MemberName; }; \
          void        set##FaceName(const Type &value) { MemberName = value; }

#define GET_PTR_SET(Type, MemberName, FaceName) \
          Type *get##FaceName() const { return MemberName; }; \
          void  set##FaceName(const Type &value) { MemberName = value; }

#define GET_PTR_SET_PTR(Type, MemberName, FaceName) \
          const Type *get##FaceName() const { return MemberName; }; \
          void  set##FaceName(Type *value) { MemberName = value; }

#define REF_PTR_SET_PTR(Type, MemberName, FaceName) \
          Type *ref##FaceName() { return MemberName; }; \
          void  set##FaceName(Type *value) { MemberName = value; }

#define GET_CPTR_SET(Type, MemberName, FaceName) \
          const Type *get##FaceName() const { return MemberName; }; \
          void        set##FaceName(const Type &value) { MemberName = value; }

#define GET_ARRAY_AS_POINTER(Type, MemberName, FaceName) \
          const Type* get##FaceName() const { return &MemberName[0]; }

#define REF_ARRAY_AS_POINTER(Type, MemberName, FaceName) \
          Type* ref##FaceName() { return &MemberName[0]; }

#endif _GETSETMACROS_H_
