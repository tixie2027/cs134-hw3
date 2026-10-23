#include "mobj.h"
#include "pframe.h"
#include "util/string.h"

/*
 * Hints:
 *   - when we write the ops, we want to make sure that we copy the value
 *     of the parameters into the space that we've been provided
 */
void mobj_create(mobj_t *o, mobj_ops_t *ops) {
    o->mo_ops = *ops;
    list_init(&o->mo_pframes);
    spinlock_init(&o->mo_mutex);
    o->parent = NULL;
}

/*
 * TODO: implement me!
 *
 * Hints:
 *   - in this function, where do we search for frame?
 *   - if we don't find the frame, set the pfp to NULL
 */
void mobj_find_pframe(mobj_t *o, long pagenum, pframe_t **pfp) {
    *pfp = NULL;

    list_link_t *link = o->mo_pframes.head;

    while (link != NULL) {
        pframe_t *cur_frame = link->parent;

        if (cur_frame->pf_pagenum == pagenum) {
            *pfp = cur_frame;
            return;
        }

        link = link->next;
    }
}

/*
 * Hints:
 *   - the frame might be in our list already...
 *   - we have access to a function to read the pframe from disk!
 *   - check how we get data from disk in sys/disk/disk.{h,c}...
 */
void mobj_get_pframe(mobj_t *o, long pagenum, pframe_t **pfp) {
    mobj_find_pframe(o, pagenum, pfp);

    if (*pfp) {
        return;
    }

    if (o->mo_ops.read_pframe(o, pagenum, pfp) != 0)
        *pfp = NULL;
}

/*
 * TODO: implement me!
 *
 * Hints:
 *   - pos describes where to read from in terms of bytes
 *   - len is the maximum number of bytes to read... when can we not read all
 *     of the way up to len?
 *   - save what we read to buf
 */
long mobj_read(mobj_t *o, long pos, long len, char *buf) {
    long read = 0;

    while (read < len) {
        long page = pos / FRAME_LEN;
        long offset = pos % FRAME_LEN;

        pframe_t *pframe;
        mobj_find_pframe(o, page, &pframe);

        if (pframe == NULL) {
            break;
        }

        buf[read] = pframe->bytes[offset];
        pos++;
        read++;
    }
    return read;
}

/*
 * TODO: implement me!
 *
 * Hints:
 *   - pos describes where to write to in terms of bytes
 *   - len is the maximum number of bytes to write... when can we not read all
 *     of the way up to len?
 *   - save what we read to buf
 */
long mobj_write(mobj_t *o, long pos, long len, const char *buf) {
    long read = 0;

    while (read < len) {
        long page = pos / FRAME_LEN;
        long offset = pos % FRAME_LEN;

        pframe_t *pf;
        mobj_find_pframe(o, page, &pf);

        if (pf == NULL) {
            break;
        }

        pf->bytes[offset] = buf[read];

        read++;
        pos++;
    }

    return read;
}

/*
 * Hints:
 *   - make sure that disk is up to date in the places that we care about!
 */
void mobj_flush(mobj_t *o) {
    list_link_t *link = o->mo_pframes.head;

    while (link != NULL) {
        pframe_t *pf = link->parent;

        if (o->mo_ops.flush_pframe(o, pf) != 0) {
            return;
        }
        link = link->next;
    }
}
