
%module ms3d
%{
        #include "shader.h"
        #include "ms3d.h"
        #include "Tex.h"
        #include "GLM.h"
        #include "Lights.h"
        #include "Shadows.h"
        #include "Text.h"
        #include "Body.h"
%}

%typemap(out) float* getJointPosition{
        int i;
        //$1, $1_dim0, $1_dim1
        $result = PyList_New(3);
        for (i=0; i<3; i++){
                PyObject* o = PyFloat_FromDouble((double) $1[i]);
                PyList_SetItem($result, i, o);
        }
} 

%typemap(out) glm::vec3 vectorRotate{
        int i;
        //$1, $1_dim0, $1_dim1
        $result = PyList_New(3);
        for (i=0; i<3; i++){
                PyObject* o = PyFloat_FromDouble((double) $1[i]);
                PyList_SetItem($result, i, o);
        }
} 

%typemap(out) glm::vec3 normalizeVector{
        int i;
        //$1, $1_dim0, $1_dim1
        $result = PyList_New(3);
        for (i=0; i<3; i++){
                PyObject* o = PyFloat_FromDouble((double) $1[i]);
                PyList_SetItem($result, i, o);
        }
} 

%include "shader.h"
%include "ms3d.h"
%include "Tex.h"
%include "GLM.h"
%include "Lights.h"
%include "Shadows.h"
%include "Text.h"
%include "Body.h"
