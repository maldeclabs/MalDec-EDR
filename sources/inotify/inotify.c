#include "inotify/inotify.h"
#include "err/err.h"
#include "logger/logger.h"

#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

ERR
init_inotify(INOTIFY **inotify, INOTIFY_CONFIG config)
{
  ERR retval = ERR_FAILURE;

  *inotify = malloc(sizeof(struct INOTIFY));
  ALLOC_ERR_FAILURE(*inotify);

  (*inotify)->config = config;

  (*inotify)->fd_inotify = inotify_init1(IN_NONBLOCK);

  if ((*inotify)->fd_inotify < 0)
  {
    LOG_ERROR(
            LOG_MESSAGE_FORMAT("ERR_FAILURE %d (%s)", errno, strerror(errno)));
    return retval;
  }

  (*inotify)->wd = calloc((*inotify)->config.quantity_fds, sizeof(int));
  ALLOC_ERR_FAILURE((*inotify)->wd);

  if (IS_NULL_PTR((*inotify)->wd))
  {
    LOG_ERROR(
            LOG_MESSAGE_FORMAT("ERR_FAILURE %d (%s)", errno, strerror(errno)));
    return retval;
  }

  struct PATHS *paths = (*inotify)->config.paths;
  for (int i = 0; i < (*inotify)->config.quantity_fds;
       paths = paths->hh.next, i++)
  {
    (*inotify)->wd[i] = inotify_add_watch((*inotify)->fd_inotify, paths->path,
                                          IN_ALL_EVENTS);
    LOG_INFO(LOG_MESSAGE_FORMAT("inotify path '%s' IN_ALL_EVENTS", paths->path));

    if ((*inotify)->wd[i] == -1)
    {
      LOG_ERROR(LOG_MESSAGE_FORMAT("ERR_FAILURE  Cannot watch '%s' %d (%s)",
                                   paths->path, errno, strerror(errno)));
      return retval;
    }
  }

  (*inotify)->nfds = 1;

  (*inotify)->fds[0].fd     = (*inotify)->fd_inotify;
  (*inotify)->fds[0].events = POLLIN;

  retval = ERR_SUCCESS;

  return retval;
}

void
listen_to_events_inotify(INOTIFY **inotify, void *user_data,
                         handles_events_t handles)
{
  while (true)
  {
    (*inotify)->poll_num = poll((*inotify)->fds, (*inotify)->nfds, -1);
    if ((*inotify)->poll_num == -1)
    {
      if (errno == EINTR) continue;
      LOG_ERROR(LOG_MESSAGE_FORMAT("ERR_FAILURE pool %d (%s)", errno,
                                   strerror(errno)));
      exit(EXIT_FAILURE);
    }

    if ((*inotify)->poll_num > 0)
    {
      if ((*inotify)->fds[0].revents & POLLIN) { handles(*inotify, user_data); }
    }
  }
}

void
exit_inotify(INOTIFY **inotify)
{
  if (!IS_NULL_PTR(*inotify))
  {
    close((*inotify)->fd_inotify);

    del_paths(&(*inotify)->config.paths);
    free((*inotify)->wd);
    free(*inotify);
  }
}