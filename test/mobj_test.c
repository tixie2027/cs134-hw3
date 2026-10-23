#include <assert.h>
#include <string.h>
#include "mobj.h"
#include "pframe.h"

int main(void) {
    mobj_t o;
    mobj_ops_t ops = {0};
    mobj_create(&o, &ops);

    pframe_t pf = {0};
    list_link_init(&pf.pf_link, &pf);
    list_insert_back(&o.mo_pframes, &pf.pf_link);

    char buf[3];
    assert(mobj_write(&o, 0, 3, "cat") == 3);
    assert(mobj_read(&o, 0, 3, buf) == 3);
    assert(memcmp(buf, "cat", 3) == 0);
}