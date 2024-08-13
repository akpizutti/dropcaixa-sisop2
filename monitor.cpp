#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <unistd.h>

#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN (1024 * (EVENT_SIZE + 16))

int main(int argc, char **argv)
{
    int length, i = 0;
    int inotifyfd;
    int inotifywd;
    char buffer[BUF_LEN];
    bool is_watching = true;

    inotifyfd = inotify_init();

    if (inotifyfd < 0)
    {
        perror("inotify_init");
    }

    while (is_watching == true)
    {

        inotifywd = inotify_add_watch(inotifyfd, "./sync_dir",
                                      IN_MODIFY | IN_CREATE | IN_DELETE);
        length = read(inotifyfd, buffer, BUF_LEN);

        if (length < 0)
        {
            perror("read");
        }

        i = 0;

        while (i < length)
        {
            struct inotify_event *event =
                (struct inotify_event *)&buffer[i];
            if (event->len)
            {
                if (event->mask & IN_CREATE)
                {
                    printf("The file %s was created.\n", event->name);
                }
                else if (event->mask & IN_DELETE)
                {
                    printf("The file %s was deleted.\n", event->name);
                }
                else if (event->mask & IN_MODIFY)
                {
                    printf("The file %s was modified.\n", event->name);
                }
            }
            i += EVENT_SIZE + event->len;
        }
    }

    (void)inotify_rm_watch(inotifyfd, inotifywd); // <- remove watch
    (void)close(inotifyfd);

    return 0;
}