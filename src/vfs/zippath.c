
#include "zippath.h"
#include "zipfile.h"
#include "syspath.h"
#include "rwops/all.h"

static const char* vfs_zippath_name(VFSNode *node) {
    VFSZipPathData *zdata = node->data;
    return zip_get_name(zdata->tls->zip, zdata->index, 0);
}

static void vfs_zippath_free(VFSNode *node) {
    free(node->data);
}

static char* vfs_zippath_repr(VFSNode *node) {
    VFSZipPathData *zdata = node->data;
    char *ziprepr = vfs_repr_node(zdata->zipnode, false);
    char *zpathrepr = strfmt("%s '%s' in %s",
        zdata->info.is_dir ? "directory" : "file", vfs_zippath_name(node), ziprepr);
    free(ziprepr);
    return zpathrepr;
}

static char* vfs_zippath_syspath(VFSNode *node) {
    VFSZipPathData *zdata = node->data;
    char *zippath = vfs_repr_node(zdata->zipnode, true);
    char *subpath = strfmt("%s%c%s", zippath, vfs_syspath_prefered_separator, vfs_zippath_name(node));
    free(zippath);
    return subpath;
}

static VFSInfo vfs_zippath_query(VFSNode *node) {
    VFSZipPathData *zdata = node->data;
    return zdata->info;
}

static VFSNode* vfs_zippath_locate(VFSNode *node, const char *path) {
    VFSZipPathData *zdata = node->data;

    const char *mypath = vfs_zippath_name(node);
    char fullpath[strlen(mypath) + strlen(path) + 2];
    snprintf(fullpath, sizeof(fullpath), "%s%c%s", mypath, VFS_PATH_SEP, path);
    vfs_path_normalize_inplace(fullpath);

    return vfs_locate(zdata->zipnode, fullpath);
}

static const char* vfs_zippath_iter(VFSNode *node, void **opaque) {
    VFSZipPathData *zdata = node->data;
    VFSZipFileIterData *idata = *opaque;

    if(!zdata->info.is_dir) {
        return NULL;
    }

    if(!idata) {
        idata = calloc(1, sizeof(VFSZipFileIterData));
        idata->num = zip_get_num_entries(zdata->tls->zip, 0);
        idata->idx = zdata->index;
        idata->prefix = vfs_zippath_name(node);
        idata->prefix_len = strlen(idata->prefix);
        *opaque = idata;
    }

    return vfs_zipfile_iter_shared(node, zdata->zipnode->data, idata, zdata->tls);
}

#define vfs_zippath_iter_stop vfs_zipfile_iter_stop

static SDL_RWops* vfs_zippath_open(VFSNode *node, VFSOpenMode mode) {
    if(mode & VFS_MODE_WRITE) {
        vfs_set_error("ZIP archives are read-only");
        return NULL;
    }

    VFSZipPathData *zdata = node->data;
    zip_file_t *zipfile = zip_fopen_index(zdata->tls->zip, zdata->index, 0);

    if(!zipfile) {
        vfs_set_error("ZIP error: %s", zip_error_strerror(&zdata->tls->error));
        return NULL;
    }

    SDL_RWops *ziprw = SDL_RWFromZipFile(zipfile, true);
    assert(ziprw != NULL);

    if(!(mode & VFS_MODE_SEEKABLE)) {
        return ziprw;
    }

    uint8_t buf[4096] = {0};
    ssize_t len;
    SDL_RWops *abufrw = SDL_RWAutoBuffer(NULL, 4096);

    while((len = SDL_RWread(ziprw, buf, 1, sizeof(buf))) > 0) {
        SDL_RWwrite(abufrw, buf, 1, len);
    }

    SDL_RWclose(ziprw);

    size_t datasize = SDL_RWtell(abufrw);
    SDL_RWseek(abufrw, 0, RW_SEEK_SET);
    abufrw = SDL_RWWrapSegment(abufrw, 0, datasize, true);

    return abufrw;
}

static VFSNodeFuncs vfs_funcs_zippath = {
    .repr = vfs_zippath_repr,
    .query = vfs_zippath_query,
    .free = vfs_zippath_free,
    .syspath = vfs_zippath_syspath,
    //.mount = vfs_zippath_mount,
    .locate = vfs_zippath_locate,
    .iter = vfs_zippath_iter,
    .iter_stop = vfs_zippath_iter_stop,
    //.mkdir = vfs_zippath_mkdir,
    .open = vfs_zippath_open,
};

void vfs_zippath_init(VFSNode *node, VFSNode *zipnode, VFSZipFileTLS *tls, zip_int64_t idx) {
    VFSZipPathData *zdata = calloc(1, sizeof(VFSZipPathData));
    zdata->zipnode = zipnode;
    zdata->tls = tls;
    zdata->index = idx;
    node->data = zdata;

    zdata->info.exists = true;

    if('/' == *(strchr(vfs_zippath_name(node), 0) - 1)) {
        zdata->info.is_dir = true;
    }

    node->type = VNODE_ZIPPATH;
    node->funcs = &vfs_funcs_zippath;

    const char *path = vfs_zippath_name(node);
    char buf[strlen(path)+1], *base, *name;
    strcpy(buf, path);
    vfs_path_split_right(buf, &base, &name);
}
