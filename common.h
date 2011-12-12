#ifndef INCLUDE_COMMON_DEFS_H
#define INCLUDE_COMMON_DEFS_H

#define VALUE_SERVICE_NAME        "uk.ac.cam.db538.volume-notification"
#define VALUE_SERVICE_OBJECT_PATH "/VolumeNotification"
/* And we're interested in using it through this interface.
   This must match the entry in the interface definition XML. */
#define VALUE_SERVICE_INTERFACE   "uk.ac.cam.db538.VolumeNotification"

void handleError(const char* msg, const char* reason, gboolean fatal);
void printDebug(const gchar *msg, int debug);
void printDebugOK(int debug);

#endif
