/*
 * This file is modified by Ivan Maidanski <ivmai@ivmaisoft.com>
 * Project name: JCGO (http://www.ivmaisoft.com/jcgo/)
 * File location: $(JCGO)/goclsp/clsp_fix/native/jni/classpath
 * Origin: GNU Classpath v0.93
 */

/* jcl.c
   Copyright (C) 1998, 2005, 2006 Free Software Foundation, Inc.

This file is part of GNU Classpath.

GNU Classpath is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GNU Classpath is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU Classpath; see the file COPYING.  If not, write to the
Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
02110-1301 USA.

Linking this library statically or dynamically with other modules is
making a combined work based on this library.  Thus, the terms and
conditions of the GNU General Public License cover the whole
combination.

As a special exception, the copyright holders of this library give you
permission to link this library with independent modules to produce an
executable, regardless of the license terms of these independent
modules, and to copy and distribute the resulting executable under
terms of your choice, provided that you also meet, for each linked
independent module, the terms and conditions of the license of that
module.  An independent module is a module which is not derived from
or based on this library.  If you modify this library, you may extend
this exception to your version of the library, but you are not
obligated to do so.  If you do not wish to do so, delete this
exception statement from your version. */

/* do not move; needed here because of some macro definitions */
#include "config.h"

#include <stdlib.h>
#include <stdio.h>
#include <jcl.h>

#if !defined(__GNUC__) && !defined(__attribute__)
#   define __attribute__(x)     /* nothing */
#endif

/*
 * Cached Pointer class info.
 */
static jclass rawDataClass = NULL;
static jfieldID rawData_fid = NULL;
static jmethodID rawData_mid = NULL;

/* JNI_OnLoad is not used here (to enable static linking) */

#if SIZEOF_VOID_P == 8
#define JCL_POINTER_CLASSNAME "gnu/classpath/Pointer64"
#define JCL_POINTER_DATASIGN "J"
#else
#if SIZEOF_VOID_P == 4
#define JCL_POINTER_CLASSNAME "gnu/classpath/Pointer32"
#define JCL_POINTER_DATASIGN "I"
#else
#error "Pointer size is not supported."
#endif /* SIZEOF_VOID_P == 4 */
#endif /* SIZEOF_VOID_P == 8 */

JNIEXPORT void JNICALL
JCL_ThrowException (JNIEnv * env, const char *className, const char *errMsg)
{
  jclass excClass;
  if ((*env)->ExceptionOccurred (env))
    {
      (*env)->ExceptionClear (env);
    }
  excClass = (*env)->FindClass (env, className);
  if (excClass == NULL)
    {
      jclass errExcClass;
      errExcClass =
        (*env)->FindClass (env, "java/lang/ClassNotFoundException");
      if (errExcClass == NULL)
        {
          errExcClass = (*env)->FindClass (env, "java/lang/InternalError");
          if (errExcClass == NULL)
            {
              fprintf (stderr, "JCL: Utterly failed to throw exeption ");
              fprintf (stderr, "%s", className);
              fprintf (stderr, " with message ");
              fprintf (stderr, "%s", errMsg);
              return;
            }
        }
      /* Removed this (more comprehensive) error string to avoid the need for
       * a static variable or allocation of a buffer for this message in this
       * (unlikely) error case. --Fridi.
       *
       * sprintf(errstr,"JCL: Failed to throw exception %s with message %s: could not find exception class.", className, errMsg);
       */
      (*env)->ThrowNew (env, errExcClass, className);
    }
  (*env)->ThrowNew (env, excClass, errMsg);
}

JNIEXPORT void *JNICALL
JCL_malloc (JNIEnv * env, size_t size)
{
  void *mem = malloc (size);
  if (mem == NULL)
    {
      JCL_ThrowException (env, "java/lang/OutOfMemoryError",
                          "malloc() failed.");
      return NULL;
    }
  return mem;
}

JNIEXPORT void *JNICALL
JCL_realloc (JNIEnv * env, void *ptr, size_t size)
{
  void *orig_ptr = ptr;
  ptr = realloc (ptr, size);
  if (ptr == NULL)
    {
      if (orig_ptr != NULL)
        free (orig_ptr);
      JCL_ThrowException (env, "java/lang/OutOfMemoryError",
                          "malloc() failed.");
    }
  return (ptr);
}

JNIEXPORT void JNICALL
JCL_free (JNIEnv * env __attribute__ ((unused)), void *p)
{
  if (p != NULL)
    {
      free (p);
    }
}

JNIEXPORT const char *JNICALL
JCL_jstring_to_cstring (JNIEnv * env, jstring s)
{
  const char *cstr;
  if (s == NULL)
    {
      JCL_ThrowException (env, "java/lang/NullPointerException",
                          "Null string");
      return NULL;
    }
  cstr = (const char *) (*env)->GetStringUTFChars (env, s, NULL);
  if (cstr == NULL)
    {
      JCL_ThrowException (env, "java/lang/InternalError",
                          "GetStringUTFChars() failed.");
      return NULL;
    }
  return cstr;
}

JNIEXPORT void JNICALL
JCL_free_cstring (JNIEnv * env, jstring s, const char *cstr)
{
  (*env)->ReleaseStringUTFChars (env, s, cstr);
}

JNIEXPORT jint JNICALL
JCL_MonitorEnter (JNIEnv * env, jobject o)
{
  jint retval = (*env)->MonitorEnter (env, o);
  if (retval != 0)
    {
      JCL_ThrowException (env, "java/lang/InternalError",
                          "MonitorEnter() failed.");
    }
  return retval;
}

JNIEXPORT jint JNICALL
JCL_MonitorExit (JNIEnv * env, jobject o)
{
  jint retval = (*env)->MonitorExit (env, o);
  if (retval != 0)
    {
      JCL_ThrowException (env, "java/lang/InternalError",
                          "MonitorExit() failed.");
    }
  return retval;
}

JNIEXPORT jclass JNICALL
JCL_FindClass (JNIEnv * env, const char *className)
{
  jclass retval = (*env)->FindClass (env, className);
  if (retval == NULL)
    {
      JCL_ThrowException (env, "java/lang/ClassNotFoundException", className);
    }
  return retval;
}


/*
 * Build a Pointer object.
 */

JNIEXPORT jobject JNICALL
JCL_NewRawDataObject (JNIEnv * env, void *data)
{
  jclass aclass = rawDataClass;
  if (aclass == NULL)
    {
      aclass = (*env)->FindClass (env, JCL_POINTER_CLASSNAME);
      if (aclass == NULL ||
          (aclass = (*env)->NewGlobalRef (env, aclass)) == NULL)
        {
          JCL_ThrowException (env, "java/lang/InternalError",
                              "Pointer class was not properly initialized");
          return NULL;
        }
      rawDataClass = aclass;
    }

  if (rawData_mid == NULL)
    {
      if ((rawData_mid = (*env)->GetMethodID (env, aclass, "<init>",
                          "(" JCL_POINTER_DATASIGN ")V")) == NULL)
        {
          JCL_ThrowException (env, "java/lang/InternalError",
                              "Pointer class was not properly initialized");
          return NULL;
        }
    }

#if SIZEOF_VOID_P == 8
  return (*env)->NewObject (env, aclass, rawData_mid,
                            (jlong) (ptrdiff_t) data);
#else
  return (*env)->NewObject (env, aclass, rawData_mid, (jint) (ptrdiff_t) data);
#endif
}

JNIEXPORT void * JNICALL
JCL_GetRawData (JNIEnv * env, jobject rawdata)
{
  if (rawData_fid == NULL)
    {
      jclass aclass = rawDataClass;
      if ((aclass == NULL && (aclass = (*env)->FindClass (env,
                                        JCL_POINTER_CLASSNAME)) == NULL) ||
          (rawData_fid = (*env)->GetFieldID (env, aclass, "data",
                                             JCL_POINTER_DATASIGN)) == NULL)
        {
          JCL_ThrowException (env, "java/lang/InternalError",
                              "Pointer class was not properly initialized");
          return NULL;
        }
    }

#if SIZEOF_VOID_P == 8
  return (void *) (ptrdiff_t) (*env)->GetLongField (env, rawdata, rawData_fid);
#else
  return (void *) (ptrdiff_t) (*env)->GetIntField (env, rawdata, rawData_fid);
#endif
}
