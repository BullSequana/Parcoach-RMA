#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "core/interval.h"
#include <inttypes.h>

/* Convert an access type to a string */
char* access_type_to_str(Access_type access)
{
    if (access == LOCAL_READ)
        return "LOCAL_READ";
    else if (access == LOCAL_WRITE)
        return "LOCAL_WRITE";
    else if (access == RMA_READ)
        return "RMA_READ";
    else // access == RMA_WRITE
        return "RMA_WRITE";
}

/* Check if the access rights are compatible. */
static inline int check_access_rights(Interval itvA, Interval itvB)
{
  /* By doing a bitwise OR on the two access types, the resulting
   * value is an error only if the two bits are at 1 (i.e. there is at
   * least a local WRITE access combined with a remote access, or a
   * remote WRITE access). */
  return ((itvA.access_type | itvB.access_type) == RMA_WRITE);
}

void print_interval(Interval itv)
{
  LOG(stderr,"Interval = [%" PRIu64 " ,%" PRIu64 "] ", itv.low_bound, itv.up_bound);
  LOG(stderr,"notif_id %d ", itv.notification_id);
  switch(itv.access_type) {
    case LOCAL_READ:
      LOG(stderr,"LOCAL READ\n");
      break;
    case LOCAL_WRITE:
      LOG(stderr,"LOCAL WRITE\n");
      break;
    case RMA_READ:
      LOG(stderr,"RMA READ\n");
      break;
    case RMA_WRITE:
      LOG(stderr,"RMA WRITE\n");
      break;
    default:
      LOG(stderr,"Unknown access type\n");
  }
}

Interval *create_interval(uint64_t low_bound, uint64_t up_bound,
                          Access_type access_type, int notif_id,
                          int line, const char* filename)
{
  Interval *itv = malloc(sizeof(Interval));
  itv->low_bound = low_bound;
  itv->up_bound = up_bound;
  itv->access_type = access_type;
  itv->notification_id = notif_id;
  itv->line = line;
  if (filename != NULL)
  {
    itv->filename[FILENAME_MAX_LENGTH - 1] = 0;
    strncpy(&(itv->filename[0]), filename, FILENAME_MAX_LENGTH);
    if (itv->filename[FILENAME_MAX_LENGTH - 1] != 0) {
      LOG(stderr,"Error setting filename, overflow detected, resetting to empty\n");
      memset(&(itv->filename[0]), 0, FILENAME_MAX_LENGTH);
    }
    itv->filename[FILENAME_MAX_LENGTH - 1] = '\0';
  }
  else {
    itv->filename[0] = '\0';
  }
  return itv;
}

void free_interval(Interval *itv)
{
  free(itv);
}

uint64_t get_low_bound(const Interval *itv)
{
  return itv->low_bound;
}

uint64_t get_up_bound(const Interval *itv)
{
  return itv->up_bound;
}

Access_type get_access_type(const Interval *itv)
{
  return itv->access_type;
}

int get_notif_id(const Interval *itv)
{
  return itv->notification_id;
}

int get_fileline(const Interval *itv)
{
  return itv->line;
}

char* get_filename(Interval *itv)
{
  return &(itv->filename[0]);
}

int if_intersects(Interval itvA, Interval itvB)
{
  // itvA.low_bound > itvB.low_bound or itvA.up_bound < itvB.low_bound
  // No intersection
  if((itvA.low_bound > itvB.up_bound) || (itvA.up_bound < itvB.low_bound))
  {
      //LOG(stderr,"Pas d'intersection !\n");
      return 0;
  }
  else
  {
    /* Several intersection cases are checked here :
       - itvB.low_bound < itvA.low_bound < itvA.up_bound < itvB.up_bound
         Intersection of A & B : A completely included in B
       - itvA.low_bound < itvB.low_bound < itvB.up_bound < itvA.up_bound
         Intersection of A & B : B completely included in A
       - itvB.low_bound < itvA.low_bound < itvB.up_bound < itvA.up_bound,
         Intersection of A & B = [itvA.low_bound ; itvB.up_bound]
       - itvA.low_bound < itvB.low_bound < itvA.up_bound < itvB.up_bound
         Intersection de A & B = [itvB.low_bound ; itvA.up_bound] */
    return check_access_rights(itvA, itvB);
  }
}
