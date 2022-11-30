/*
 * Mesh Loader v1.0 Header
 * Date: 21 November, 2022
 * Author: Kaivalya Deshpande
 */

#ifndef _MESHLOADER_H_
#define _MESHLOADER_H_

#ifdef __cplusplus
extern "C" {
#endif

/* headers */
#include<stdio.h>    // for file I/O
#include<stdlib.h>   // for ato*()
#include<string.h>   // for strtok_r()
#include<sys/stat.h> // for stat() and struct stat
#include<malloc.h>   // for malloc()

#include<GL/glew.h>
#include<GL/gl.h>

#include"cvector/cvector.h"

/* enums */
enum {
    MESH_SUCCESS = 0,
    MESH_ERR_UNKNOWNFILESIZE = 1,
    MESH_ERR_OUTOFMEMORY,
    MESH_ERR_FILEUNREAD,
    MESH_FAILURE
};

/* struct definitions */
struct mesh_t
{
    struct fvec *positions;
    struct fvec *texCoords;
    struct fvec *normals;
    struct ivec *faces;

    int vertex_count;
    int face_count;
};

struct opengl_compatible_mesh_t
{
    GLuint vao;
    GLuint vboPerVertexData;
};

/* Generic mesh loading */
void mesh_create(struct mesh_t *mesh);
int mesh_load_from_obj(struct mesh_t *mesh, const char *meshfile);
void mesh_destroy(struct mesh_t *mesh);

/* 
 * A Simplifying API-Layer to automate mesh loading for OpenGL
 *
 * It is assumed that an OpenGL core profile context
 * of version >= 3.0 has already been fetched and made
 * current by users in case they wish to make use of
 * the simplification layer for OpenGL exposed by this
 * library
 * 
 * OPENGL VERSION IS NOT CHECKED INTERNALLY!
 */
void mesh_set_opengl_attrib_location_position(GLuint location);
void mesh_set_opengl_attrib_location_normal(GLuint location);
void mesh_set_opengl_attrib_location_texcoord(GLuint location);
int mesh_create_opengl_compatible_mesh(struct mesh_t *mesh, struct opengl_compatible_mesh_t *ogl_mesh);
void mesh_destroy_opengl_compatible_mesh(struct opengl_compatible_mesh_t *ogl_mesh);

#ifdef __cplusplus
}
#endif

#endif  // _MESHLOADER_H_
