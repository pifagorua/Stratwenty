#include <object3d.h>

Object3d::Object3d()
{

}

Object3d::~Object3d()
{
    mainTex.unloadTex();
    glDeleteBuffers(1, &m_buffer);
    glDeleteBuffers(1, &m_uvbuff);
}

void Object3d::draw(GLuint shader_program, glm::mat4 &cam_matrix)
{
    glUseProgram(shader_program);

    MoveMatrix   = glm::translate(glm::mat4(), pos);
    RotateMatrix = glm::rotate(glm::mat4(), rot.x, glm::vec3(1,0,0))*glm::rotate(glm::mat4(), rot.y, glm::vec3(0,1,0))*glm::rotate(glm::mat4(), rot.z, glm::vec3(0,0,1));
    ScaleMatrix  = glm::mat4(1);

    m_matrix = cam_matrix * MoveMatrix * RotateMatrix * ScaleMatrix;

    m_matid = glGetUniformLocation(shader_program, "MVP");
    glUniformMatrix4fv(m_matid, 1, GL_FALSE, &m_matrix[0][0]);


    glEnableVertexAttribArray(0); // ID=0
    glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0); // ID=0

    glEnableVertexAttribArray(1); // ID=1
    glBindBuffer(GL_ARRAY_BUFFER, m_uvbuff); // Texture UV coordinates
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0); // ID=1


    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_texid);
    glUniform1i(glGetUniformLocation(shader_program, "mainTexture"), 0);

    glDrawArrays(GL_TRIANGLES, 0, m_size); // Начиная с вершины 0, всего 3 вершины -> один треугольник

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}

void Object3d::setTex(unistring fname)
{
    mainTex.loadTex(fname);

    if(!mainTex.data)
    {
        printf("Error while loading texture: %s\n", fname.c_str());
        return;
    }
}

void Object3d::update(const vector<GLfloat> &verts, int draw_type)
{
    if(m_VAO == 0)
        glGenVertexArrays(1, &m_VAO);
    if(m_buffer == 0)
        glGenBuffers(1, &m_buffer);
    if(m_texid == 0)
        glGenTextures(1, &m_texid);

    vertices = verts;

    m_size = vertices.size();
    GLfloat data[m_size];
    for(int i=0; i < m_size; i++)
        data[i] = verts.at(i);

    glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, draw_type);
}

void Object3d::update(int draw_type, unistring targetTex)
{
    GLfloat uvdata[texCoords.size()];
    for(int i=0; i < texCoords.size(); i++)
       uvdata[i] = texCoords.at(i);
    glBindBuffer(GL_ARRAY_BUFFER, m_uvbuff);
    glBufferData(GL_ARRAY_BUFFER, sizeof(uvdata), uvdata, draw_type);

    glBindTexture(GL_TEXTURE_2D, m_texid);
    if(targetTex == "none")
    {
        uchar checkImage[TEX_SIZE][TEX_SIZE][RGBAC]; // R,G,B,A = 4
        // Create purple-black checker
        int i, j;
        bool c=true;

        for (i = 0; i < TEX_SIZE; i++) {
            for (j = 0; j < TEX_SIZE; j++) {
                checkImage[i][j][0] = (uchar) c == true ? 255: 0;
                checkImage[i][j][1] = (uchar)0;
                checkImage[i][j][2] = (uchar) c == true ? 255: 0;
                checkImage[i][j][3] = (uchar) 255;

                if(j == TEX_SIZE-1)
                {
                    if(c) // if purple
                        c = true;
                    else // if black
                        c = false;
                }
                else
                    c = !c;
            }
        }

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TEX_SIZE, TEX_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, &checkImage);
    }
    else
    {
        setTex(unistring(IMG_ROOT) + targetTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mainTex.width, mainTex.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, mainTex.data);
        mainTex.unloadTex();
    }

    glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glGenerateMipmap(GL_TEXTURE_2D);
}

void Object3d::move(glm::vec3 mv)
{
    pos += mv;
}

void Object3d::rotate(glm::vec3 axis, float angle)
{
    rot += glm::normalize(axis) * glm::radians(angle);

    RotateMatrix = glm::rotate(glm::mat4(), rot.x, glm::vec3(1,0,0)) * glm::rotate(glm::mat4(), rot.y, glm::vec3(0,1,0)) * glm::rotate(glm::mat4(), rot.z, glm::vec3(0,0,1));
}
