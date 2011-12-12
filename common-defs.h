#ifndef INCLUDE_COMMON_DEFS_H
#define INCLUDE_COMMON_DEFS_H
/**
 * This maemo code example is licensed under a MIT-style license,
 * that can be found in the file called "License" in the same
 * directory as this file.
 * Copyright (c) 2007-2008 Nokia Corporation. All rights reserved.
 *
 * This file includes the common symbolic defines for both client and
 * the server. Normally this kind of information would be part of the
 * object usage documentation, but in this example we take the easy
 * way out.
 *
 * To re-iterate: You could just as easily use strings in both client
 *                and server, and that would be the more common way.
 */

/* Well-known name for this service. */
#define VALUE_SERVICE_NAME        "uk.ac.cam.db538.volume-notification"
/* Object path to the provided object. */
#define VALUE_SERVICE_OBJECT_PATH "/VolumeNotification"
/* And we're interested in using it through this interface.
   This must match the entry in the interface definition XML. */
#define VALUE_SERVICE_INTERFACE   "uk.ac.cam.db538.VolumeNotification"

#endif
