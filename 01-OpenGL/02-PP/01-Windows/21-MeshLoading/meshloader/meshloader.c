/*
 * Mesh Loader v1.1 Implementation
 * Date: 30 November, 2022
 * Author: Kaivalya Deshpande
 */

#include"meshloader.h"

/* unexposed struct definitions */
struct _ogl_attrib_locations {
    GLuint position;
    GLuint normal;
    GLuint texCoord;
} ogl_attribute_locations;

//////////////////////////////////////////////////////////////////////
//                implementing a generic mesh type                  //
//////////////////////////////////////////////////////////////////////
void mesh_create(struct mesh_t *mesh)
{
    mesh->positions = fvec_create();
    mesh->texCoords = fvec_create();
    mesh->normals = fvec_create();
    mesh->faces = ivec_create();

    mesh->vertex_count = 0;
    mesh->face_count = 0;
}

int mesh_load_from_obj(struct mesh_t *mesh, const char *meshfile)
{
    // local variables
    FILE *meshFile = NULL;
    size_t meshFileSize = 0;
    int ret;

    char *buf = NULL;
    char *line = NULL;
    char *data = NULL;
    char *line_save_ptr = NULL, *data_save_ptr = NULL;

    // opening file to check file size
    meshFile = fopen(meshfile, "r");
    {
        while ((ret = fgetc(meshFile) != EOF))
            meshFileSize++;

        buf = (char *)malloc(sizeof(char) * meshFileSize);
        if (!buf)
        {
            return MESH_ERR_OUTOFMEMORY;
        }
    }
    fclose(meshFile);

    // opening file for parsing
    meshFile = fopen(meshfile, "r");
    {
        if((ret = fread(buf, sizeof(char), meshFileSize, meshFile)) != meshFileSize)
        {
            return MESH_ERR_FILEUNREAD;
        }

        line = strtok_r(buf, "\n", &line_save_ptr);
        while(line != NULL)
        {
            switch(line[0])
            {
            case 'v':
                switch(line[1])
                {
                    case ' ':
                        data = strtok_r((line + 1), " ", &data_save_ptr);
                        fvec_push_back(mesh->positions, (float)atof(data));

                        data = strtok_r(NULL, " ", &data_save_ptr);
                        fvec_push_back(mesh->positions, (float)atof(data));

                        data = strtok_r(NULL, " ", &data_save_ptr);
                        fvec_push_back(mesh->positions, (float)atof(data));

                        mesh->vertex_count++;
                        break;

                    case 't':
                        data = strtok_r((line + 2), " ", &data_save_ptr);
                        fvec_push_back(mesh->texCoords, (float)atof(data));

                        data = strtok_r(NULL, " ", &data_save_ptr);
                        fvec_push_back(mesh->texCoords, (float)atof(data));
                        break;

                    case 'n':
                        data = strtok_r((line + 2), " ", &data_save_ptr);
                        fvec_push_back(mesh->normals, (float)atof(data));

                        data = strtok_r(NULL, " ", &data_save_ptr);
                        fvec_push_back(mesh->normals, (float)atof(data));

                        data = strtok_r(NULL, " ", &data_save_ptr);
                        fvec_push_back(mesh->normals, (float)atof(data));
                        break;

                    default:
                        // sequence starters other than {"v", "vt", "vn", "f"} are ignored
                        break;
                }
                break;
                
            case 'f':
                // triangle vertex 1 //
                data = strtok_r((line + 1), " /", &data_save_ptr);
                ivec_push_back(mesh->faces, atoi(data));

                data = strtok_r(NULL, " /", &data_save_ptr);
                ivec_push_back(mesh->faces, atoi(data));

                data = strtok_r(NULL, " /", &data_save_ptr);
                ivec_push_back(mesh->faces, atoi(data));

                // triangle vertex 2 //
                data = strtok_r(NULL, " /", &data_save_ptr);
                ivec_push_back(mesh->faces, atoi(data));

                data = strtok_r(NULL, " /", &data_save_ptr);
                ivec_push_back(mesh->faces, atoi(data));

                data = strtok_r(NULL, " /", &data_save_ptr);
                ivec_push_back(mesh->faces, atoi(data));

                // triangle vertex 3 //
                data = strtok_r(NULL, " /", &data_save_ptr);
                ivec_push_back(mesh->faces, atoi(data));

                data = strtok_r(NULL, " /", &data_save_ptr);
                ivec_push_back(mesh->faces, atoi(data));

                data = strtok_r(NULL, " /", &data_save_ptr);
                ivec_push_back(mesh->faces, atoi(data));

                mesh->face_count++;
                break;

            default:
                // sequence starters other than {"v", "vt", "vn", "f"} are ignored
                break;
            }

            // go to next line
            line = strtok_r(NULL, "\n", &line_save_ptr);
        }
    }
    fclose(meshFile);
    
    free(buf);
    buf = NULL;

    return MESH_SUCCESS;
}

void mesh_destroy(struct mesh_t *mesh)
{
    if(mesh->faces)
    {
        ivec_destroy(mesh->faces);
        mesh->faces = NULL;
    }
    
    if(mesh->normals)
    {
        fvec_destroy(mesh->normals);
        mesh->normals = NULL;
    }
    
    if(mesh->texCoords)
    {
        fvec_destroy(mesh->texCoords);
        mesh->texCoords = NULL;
    }
    
    if(mesh->positions)
    {
        fvec_destroy(mesh->positions);
        mesh->positions = NULL;
    }

    mesh->vertex_count = 0;
    mesh->face_count = 0;
}

//////////////////////////////////////////////////////////////////////
//         A simplifying API-layer for OpenGL programmers           //
//////////////////////////////////////////////////////////////////////
void mesh_set_opengl_attrib_location_position(GLuint location)
{
    // code
    ogl_attribute_locations.position = location;
}

void mesh_set_opengl_attrib_location_normal(GLuint location)
{
    // code
    ogl_attribute_locations.normal = location;
}

void mesh_set_opengl_attrib_location_texcoord(GLuint location)
{
    // code
    ogl_attribute_locations.texCoord = location;
}

int mesh_create_opengl_compatible_mesh(struct mesh_t *mesh, struct opengl_compatible_mesh_t *ogl_mesh)
{
    // local variables
    struct fvec *per_vertex_data = NULL;

    GLuint vao;
    GLuint vboPerVertexData;
    int pIndex, nIndex, tIndex, faceIndex;

    // code
    // allocate a temporary per-vertex data container
    per_vertex_data = fvec_create();

    for(faceIndex = 0; faceIndex < mesh->faces->occupied_size; faceIndex += 9)
    {
        /* first vertex in face (obj files are 1-indexed, so subtract 1 for 0-indexed vertex-data indices) */
        pIndex = (mesh->faces->pi[faceIndex + 0] - 1) * 3;  // position index
        tIndex = (mesh->faces->pi[faceIndex + 1] - 1) * 2;  // texCoord index
        nIndex = (mesh->faces->pi[faceIndex + 2] - 1) * 3;  // normal index

        // push 3 + 3 + 2 = 8 floating point values for the first vertex of this face
        fvec_push_back(per_vertex_data, mesh->positions->pf[pIndex + 0]);
        fvec_push_back(per_vertex_data, mesh->positions->pf[pIndex + 1]);
        fvec_push_back(per_vertex_data, mesh->positions->pf[pIndex + 2]);

        fvec_push_back(per_vertex_data, mesh->normals->pf[nIndex + 0]);
        fvec_push_back(per_vertex_data, mesh->normals->pf[nIndex + 1]);
        fvec_push_back(per_vertex_data, mesh->normals->pf[nIndex + 2]);

        fvec_push_back(per_vertex_data, mesh->texCoords->pf[tIndex + 0]);
        fvec_push_back(per_vertex_data, mesh->texCoords->pf[tIndex + 1]);

        /* second vertex in face */
        pIndex = (mesh->faces->pi[faceIndex + 3] - 1) * 3;  // position index
        tIndex = (mesh->faces->pi[faceIndex + 4] - 1) * 2;  // texCoord index
        nIndex = (mesh->faces->pi[faceIndex + 5] - 1) * 3;  // normal index

        // push 3 + 3 + 2 = 8 floating point values for the second vertex of this face
        fvec_push_back(per_vertex_data, mesh->positions->pf[pIndex + 0]);
        fvec_push_back(per_vertex_data, mesh->positions->pf[pIndex + 1]);
        fvec_push_back(per_vertex_data, mesh->positions->pf[pIndex + 2]);

        fvec_push_back(per_vertex_data, mesh->normals->pf[nIndex + 0]);
        fvec_push_back(per_vertex_data, mesh->normals->pf[nIndex + 1]);
        fvec_push_back(per_vertex_data, mesh->normals->pf[nIndex + 2]);

        fvec_push_back(per_vertex_data, mesh->texCoords->pf[tIndex + 0]);
        fvec_push_back(per_vertex_data, mesh->texCoords->pf[tIndex + 1]);

        /* third vertex in face */
        pIndex = (mesh->faces->pi[faceIndex + 6] - 1) * 3;  // position index
        tIndex = (mesh->faces->pi[faceIndex + 7] - 1) * 2;  // texCoord index
        nIndex = (mesh->faces->pi[faceIndex + 8] - 1) * 3;  // normal index

        // push 3 + 3 + 2 = 8 floating point values for the third vertex of this face
        fvec_push_back(per_vertex_data, mesh->positions->pf[pIndex + 0]);
        fvec_push_back(per_vertex_data, mesh->positions->pf[pIndex + 1]);
        fvec_push_back(per_vertex_data, mesh->positions->pf[pIndex + 2]);

        fvec_push_back(per_vertex_data, mesh->normals->pf[nIndex + 0]);
        fvec_push_back(per_vertex_data, mesh->normals->pf[nIndex + 1]);
        fvec_push_back(per_vertex_data, mesh->normals->pf[nIndex + 2]);

        fvec_push_back(per_vertex_data, mesh->texCoords->pf[tIndex + 0]);
        fvec_push_back(per_vertex_data, mesh->texCoords->pf[tIndex + 1]);
    }

    // copy data to opengl buffers
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    {
        glGenBuffers(1, &vboPerVertexData);
        glBindBuffer(GL_ARRAY_BUFFER, vboPerVertexData);
        {
            glBufferData(GL_ARRAY_BUFFER, per_vertex_data->occupied_size * sizeof(float), per_vertex_data->pf, GL_STATIC_DRAW);
            glVertexAttribPointer(ogl_attribute_locations.position, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
            glVertexAttribPointer(ogl_attribute_locations.normal, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
            glVertexAttribPointer(ogl_attribute_locations.texCoord, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
        
            glEnableVertexAttribArray(ogl_attribute_locations.position);
            glEnableVertexAttribArray(ogl_attribute_locations.normal);
            glEnableVertexAttribArray(ogl_attribute_locations.texCoord);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0U);
    }
    glBindVertexArray(0U);

    // free the temporary per-vertex data container
    fvec_destroy(per_vertex_data);
    per_vertex_data = NULL;

    ogl_mesh->vao = vao;
    ogl_mesh->vboPerVertexData = vboPerVertexData;

    return 0;
}

void mesh_destroy_opengl_compatible_mesh(struct opengl_compatible_mesh_t *ogl_mesh)
{
    if(ogl_mesh->vboPerVertexData > 0U)
    {
        glDeleteBuffers(1, &(ogl_mesh->vboPerVertexData));
        ogl_mesh->vboPerVertexData = 0U;
    }

    if(ogl_mesh->vao > 0U)
    {
        glDeleteVertexArrays(1, &(ogl_mesh->vao));
        ogl_mesh->vao = 0U;
    }
}
