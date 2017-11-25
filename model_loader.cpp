#include "model_loader.h"

namespace model_loader {


void LoadModel(const char *filename, const char *mesh_name, Ogre::Root *root){

    // First load model into memory. If that goes well, we transfer the
    // mesh to OGRE
    Mesh mesh;

    // Parse file
    // Open file
    std::ifstream f;
    f.open(filename);
    if (f.fail()){
        throw(std::ios_base::failure(std::string("Error opening file ")+std::string(filename)));
    }

    // Parse lines
    std::string line;
    std::string ignore(" \t\r\n");
    std::string part_separator(" \t");
    std::string face_separator("/");
    bool added_normal = false;
    while (std::getline(f, line)){
        // Clean extremities of the string
        string_trim(line, ignore);
        // Ignore comments
        if ((line.size() <= 0) ||
            (line[0] == '#')){
            continue;
        }
        // Parse string
        std::vector<std::string> part = string_split(line, part_separator);
        // Check commands
        if (!part[0].compare(std::string("v"))){
            if (part.size() >= 4){
                Ogre::Vector3 position = Ogre::Vector3(str_to_num<float>(part[1].c_str()), str_to_num<float>(part[2].c_str()), str_to_num<float>(part[3].c_str()));
                mesh.position.push_back(position);
            } else {
                throw(std::ios_base::failure(std::string("Error: v command should have exactly 3 parameters")));
            }
        } else if (!part[0].compare(std::string("vn"))){
            if (part.size() >= 4){
                Ogre::Vector3 normal = Ogre::Vector3(str_to_num<float>(part[1].c_str()), str_to_num<float>(part[2].c_str()), str_to_num<float>(part[3].c_str()));
                mesh.normal.push_back(normal);
                added_normal = true;
            } else {
                throw(std::ios_base::failure(std::string("Error: vn command should have exactly 3 parameters")));
            }
        } else if (!part[0].compare(std::string("vt"))){
            if (part.size() >= 3){
                Ogre::Vector2 tex_coord = Ogre::Vector2(str_to_num<float>(part[1].c_str()), str_to_num<float>(part[2].c_str()));
                mesh.tex_coord.push_back(tex_coord);
            } else {
                throw(std::ios_base::failure(std::string("Error: vt command should have exactly 2 parameters")));
            }
        } else if (!part[0].compare(std::string("f"))){
            if (part.size() >= 4){
                if (part.size() > 5){
                    throw(std::ios_base::failure(std::string("Error: f commands with more than 4 vertices not supported")));
                } else if (part.size() == 5){
                    // Break a quad into two triangles
                    Quad quad;
                    for (int i = 0; i < 4; i++){
                        std::vector<std::string> fd = string_split_once(part[i+1], face_separator);
                        if (fd.size() == 1){
                            quad.i[i] = str_to_num<float>(fd[0].c_str())-1;
                            quad.t[i] = -1;
                            quad.n[i] = -1;
                        } else if (fd.size() == 2){
                            quad.i[i] = str_to_num<float>(fd[0].c_str())-1;
                            quad.t[i] = str_to_num<float>(fd[1].c_str())-1;
                            quad.n[i] = -1;
                        } else if (fd.size() == 3){
                            quad.i[i] = str_to_num<float>(fd[0].c_str())-1;
                            if (std::string("").compare(fd[1]) != 0){
                                quad.t[i] = str_to_num<float>(fd[1].c_str())-1;
                            } else {
                                quad.t[i] = -1;
                            }
                            quad.n[i] = str_to_num<float>(fd[2].c_str())-1;
                        } else {
                            throw(std::ios_base::failure(std::string("Error: f parameter should have 1 or 3 parameters separated by '/'")));
                        }
                    }
                    Face face1, face2;
                    face1.i[0] = quad.i[0]; face1.i[1] = quad.i[1]; face1.i[2] = quad.i[2];
                    face1.n[0] = quad.n[0]; face1.n[1] = quad.n[1]; face1.n[2] = quad.n[2];
                    face1.t[0] = quad.t[0]; face1.t[1] = quad.t[1]; face1.t[2] = quad.t[2];
                    face2.i[0] = quad.i[0]; face2.i[1] = quad.i[2]; face2.i[2] = quad.i[3];
                    face2.n[0] = quad.n[0]; face2.n[1] = quad.n[2]; face2.n[2] = quad.n[3];
                    face2.t[0] = quad.t[0]; face2.t[1] = quad.t[2]; face2.t[2] = quad.t[3];
                    mesh.face.push_back(face1);
                    mesh.face.push_back(face2);
                } else if (part.size() == 4){
                    Face face;
                    for (int i = 0; i < 3; i++){
                        std::vector<std::string> fd = string_split_once(part[i+1], face_separator);
                        if (fd.size() == 1){
                            face.i[i] = str_to_num<float>(fd[0].c_str())-1;
                            face.t[i] = -1;
                            face.n[i] = -1;
                        } else if (fd.size() == 2){
                            face.i[i] = str_to_num<float>(fd[0].c_str())-1;
                            face.t[i] = str_to_num<float>(fd[1].c_str())-1;
                            face.n[i] = -1;
                        } else if (fd.size() == 3){
                            face.i[i] = str_to_num<float>(fd[0].c_str())-1;
                            if (std::string("").compare(fd[1]) != 0){
                                face.t[i] = str_to_num<float>(fd[1].c_str())-1;
                            } else {
                                face.t[i] = -1;
                            }
                            face.n[i] = str_to_num<float>(fd[2].c_str())-1;
                        } else {
                            throw(std::ios_base::failure(std::string("Error: f parameter should have 1, 2, or 3 parameters separated by '/'")));
                        }
                    }
                    mesh.face.push_back(face);
                }
            } else {
                throw(std::ios_base::failure(std::string("Error: f command should have 3 or 4 parameters")));
            }
        }
        // Ignore other commands
    }

    // Close file
    f.close();

    // Check if vertex references are correct
    for (unsigned int i = 0; i < mesh.face.size(); i++){
        for (int j = 0; j < 3; j++){
            if (mesh.face[i].i[j] >= mesh.position.size()){
                throw(std::ios_base::failure(std::string("Error: index for triangle ")+num_to_str<int>(mesh.face[i].i[j])+std::string(" is out of bounds")));
            }
        }
    }

    // Compute degree of each vertex
    std::vector<int> degree(mesh.position.size(), 0);
    for (unsigned int i = 0; i < mesh.face.size(); i++){
        for (int j = 0; j < 3; j++){
            degree[mesh.face[i].i[j]]++;
        }
    }

    // Compute vertex normals if no normals were ever added
    if (!added_normal){
        mesh.normal = std::vector<Ogre::Vector3>(mesh.position.size(), Ogre::Vector3(0.0, 0.0, 0.0));
        for (unsigned int i = 0; i < mesh.face.size(); i++){
            // Compute face normal
            Ogre::Vector3 vec1, vec2;
            vec1 = mesh.position[mesh.face[i].i[0]] -
                        mesh.position[mesh.face[i].i[1]];
            vec2 = mesh.position[mesh.face[i].i[0]] -
                        mesh.position[mesh.face[i].i[2]];
            Ogre::Vector3 norm = vec1.crossProduct(vec2);
            norm.normalise();
            // Add face normal to vertices
            mesh.normal[mesh.face[i].i[0]] += norm;
            mesh.normal[mesh.face[i].i[1]] += norm;
            mesh.normal[mesh.face[i].i[2]] += norm;
        }
        for (unsigned int i = 0; i < mesh.normal.size(); i++){
            if (degree[i] > 0){
                mesh.normal[i] /= degree[i];
            }
        }
    }

    // Debug
    //print_mesh(mesh);

    // If we got to this point, the file was parsed successfully and the
    // mesh is in memory
    // Transfer the mesh to OGRE

    try {
        /* Retrieve scene manager */
        Ogre::SceneManager* scene_manager = root->getSceneManager("MySceneManager");

        /* Create the 3D object */
        Ogre::ManualObject* object = NULL;
        object = scene_manager->createManualObject(mesh_name);
        object->setDynamic(false);

        /* Create triangle list for the object */
        object->begin("", Ogre::RenderOperation::OT_TRIANGLE_LIST);

        /* Add vertices and triangles to the object */
        /* Repeat vertices for each triangle. This is necessary because
         * of the way OGRE associates normals and texture coordinates to
         * vertices */
        unsigned int vertex_index = 0;
        for (unsigned int i = 0; i < mesh.face.size(); i++){
            for (int j = 0; j < 3; j++){
                object->position(mesh.position[mesh.face[i].i[j]]);
                if (!added_normal){
                    object->normal(mesh.normal[mesh.face[i].i[j]]);
                } else {
                    if (mesh.face[i].n[j] >= 0){
                        object->normal(mesh.normal[mesh.face[i].n[j]]);
                    }
                }
                if (mesh.face[i].t[j] >= 0){
                    object->textureCoord(mesh.tex_coord[mesh.face[i].t[j]]);
                } else {
                    object->textureCoord(Ogre::Vector2(0.0, 0.0));
                }
            }
            object->triangle(vertex_index, vertex_index + 1, vertex_index + 2);
            vertex_index += 3;
        }
        
        /* We finished the object */
        object->end();
        
        /* Convert triangle list to a mesh */
        object->convertToMesh(mesh_name);
    }
    catch (Ogre::Exception &e){
        throw e;
    }
    catch(std::exception &e){
        throw e;
    }
}


void string_trim(std::string str, std::string to_trim){

    // Trim any character in to_trim from the beginning of the string str
    while ((str.size() > 0) && 
           (to_trim.find(str[0]) != std::string::npos)){
        str.erase(0);
    }

    // Trim any character in to_trim from the end of the string str
    while ((str.size() > 0) && 
           (to_trim.find(str[str.size()-1]) != std::string::npos)){
        str.erase(str.size()-1);
    }
}


std::vector<std::string> string_split(std::string str, std::string separator){

    // Initialize output
    std::vector<std::string> output;
    output.push_back(std::string(""));
    int string_index = 0;

    // Analyze string
    unsigned int i = 0;
    while (i < str.size()){
        // Check if character i is a separator
        if (separator.find(str[i]) != std::string::npos){
            // Split string
            string_index++;
            output.push_back(std::string(""));
            // Skip separators
            while ((i < str.size()) && (separator.find(str[i]) != std::string::npos)){
                i++;
            }
        } else {
            // Otherwise, copy string
            output[string_index] += str[i];
            i++;
        }
    }

    return output;
}


std::vector<std::string> string_split_once(std::string str, std::string separator){

    // Initialize output
    std::vector<std::string> output;
    output.push_back(std::string(""));
    int string_index = 0;

    // Analyze string
    unsigned int i = 0;
    while (i < str.size()){
        // Check if character i is a separator
        if (separator.find(str[i]) != std::string::npos){
            // Split string
            string_index++;
            output.push_back(std::string(""));
            // Skip single separator
            i++;
        } else {
            // Otherwise, copy string
            output[string_index] += str[i];
            i++;
        }
    }

    return output;
}


void print_mesh(Mesh &mesh){

    for (unsigned int i = 0; i < mesh.position.size(); i++){
        std::cout << "v " << 
            mesh.position[i].x << " " <<
            mesh.position[i].y << " " <<
            mesh.position[i].z << std::endl;
    }
    for (unsigned int i = 0; i < mesh.normal.size(); i++){
        std::cout << "vn " <<
            mesh.normal[i].x << " " <<
            mesh.normal[i].y << " " <<
            mesh.normal[i].z << std::endl;
    }
    for (unsigned int i = 0; i < mesh.tex_coord.size(); i++){
        std::cout << "vt " <<
            mesh.tex_coord[i].x << " " <<
            mesh.tex_coord[i].y << std::endl;
    }
    for (unsigned int i = 0; i < mesh.face.size(); i++){
        std::cout << "f " << 
            mesh.face[i].i[0] << " " <<
            mesh.face[i].i[1] << " " <<
            mesh.face[i].i[2] << " " << std::endl;
    }
}


template <typename T> std::string num_to_str(T num){

    std::ostringstream ss;
    ss << num;
    return ss.str();
}


template <typename T> T str_to_num(const std::string &str){

    std::istringstream ss(str);
    T result;
    ss >> result;
    if (ss.fail()){
        throw(std::ios_base::failure(std::string("Invalid number: ")+str));
    }
    return result;
}


} // namespace model_loader;
