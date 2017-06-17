#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/rotate_normalized_axis.hpp>
#include <GL/glew.h>
#include <stack>
#include "GLM.h"
#include "shader.h"

GLM::GLM(shader* shader){
	_Model = glm::mat4(1.0f);
	_View = glm::mat4(1.0f);
	_Projection = glm::mat4(1.0f);
	changeShader(shader);	
	
	//Get the identities loaded up...
	glUniformMatrix4fv(_modelID, 1, GL_FALSE, glm::value_ptr(_Model));
	glUniformMatrix4fv(_viewID, 1, GL_FALSE, glm::value_ptr(_View));
	//glUniformMatrix4fv(_projectionID, 1, GL_FALSE, glm::value_ptr(_Projection));
	uploadMVP();

	_activeMatrix = &_Projection;
}


GLM::~GLM(){
	//Maybe do something here?
}

void GLM::changeShader(shader* newShader){
	_shader = newShader;
	_modelID = glGetUniformLocation( _shader->getShader(), "Model" );
	_viewID = glGetUniformLocation( _shader->getShader(), "View" );
	//_projectionID = glGetUniformLocation( _shader->getShader(), "Projection");
	

	_MVPID = glGetUniformLocation( _shader->getShader(), "MVP");
}

void GLM::selectMatrix(int Matrix){
	if(Matrix == MODEL)
		_activeMatrix = &_Model;
	else if(Matrix == VIEW)
		_activeMatrix = &_View;
	else if(Matrix == PROJECTION)
		_activeMatrix = &_Projection;
	else
		return;
	_activeMatrixID = Matrix;
}

void GLM::perspective(double fov_degrees, double aspect_ratio, double near, double far){
	_Projection = glm::perspective(glm::radians(fov_degrees), aspect_ratio, near, far);
	uploadMVP();
}

void GLM::otho(double left, double right, double bottom, double top, double near, double far){
	_Projection = glm::ortho(left, right, bottom, top, near, far);
	uploadMVP();
}

void GLM::lookAt(double eyeX, double eyeY, double eyeZ, double centerX, double centerY, double centerZ, double upX, double upY, double upZ){
	_View = glm::lookAt(glm::vec3(eyeX, eyeY, eyeZ), glm::vec3(centerX, centerY, centerZ), glm::vec3(upX, upY, upZ));
	glUniformMatrix4fv(_viewID, 1, GL_FALSE, glm::value_ptr(_View));
}

void GLM::loadIdentity(){
	*(_activeMatrix) = glm::mat4(1.0f);
	uploadMatrix();
}

void GLM::translate(double x, double y, double z){
	*(_activeMatrix) = glm::translate(*(_activeMatrix), glm::vec3(x, y, z));
	uploadMatrix();
}
void GLM::rotate(float angle, double x, double y, double z){
	*(_activeMatrix) = glm::rotate(*(_activeMatrix), glm::radians(angle), glm::vec3(x, y, z));
	uploadMatrix();
}

void GLM::rotateNormalizedAxis(float angle, double x, double y, double z){
	*(_activeMatrix) = glm::rotateNormalizedAxis(*(_activeMatrix), glm::radians(angle), glm::vec3(x, y, z));
	uploadMatrix();
}


void GLM::scale(double x, double y, double z){
	*(_activeMatrix) = glm::scale(*(_activeMatrix), glm::vec3(x, y, z));
	uploadMatrix();
}

void GLM::billboard(double x, double y, double z){
	//Transpose the rotation of the View matrix (effectively its inverse)

	_Model[0][0] = _View[0][0];
	_Model[1][0] = _View[0][1];
	_Model[2][0] = _View[0][2];
	_Model[3][0] = x;

	_Model[0][1] = _View[1][0];
	_Model[1][1] = _View[1][1];
	_Model[2][1] = _View[1][2];
	_Model[3][1] = y;
	
	_Model[0][2] = _View[2][0];
	_Model[1][2] = _View[2][1];
	_Model[2][2] = _View[2][2];
	_Model[3][2] = z;

	_Model[0][3] = 0;
	_Model[1][3] = 0;
	_Model[2][3] = 0;
	_Model[3][3] = 1;

	uploadMatrix();
}

void GLM::pushMatrix(){
	_stack.push(*(_activeMatrix));
}

void GLM::popMatrix(){
	*(_activeMatrix) = _stack.top();
	_stack.pop();
	uploadMatrix();
}

void GLM::uploadMatrix(){
	if(_activeMatrixID == MODEL)
		glUniformMatrix4fv(_modelID, 1, GL_FALSE, glm::value_ptr(_Model));
	else if(_activeMatrixID == VIEW)
		glUniformMatrix4fv(_viewID, 1, GL_FALSE, glm::value_ptr(_View));
	uploadMVP();
}

void GLM::uploadMVP(){
	glUniformMatrix4fv(_MVPID, 1, GL_FALSE, glm::value_ptr(getMVP()));
}

void GLM::multiply(glm::mat4 mat){
	*(_activeMatrix) *= mat;
	uploadMatrix();
}

glm::mat4 GLM::getMVP(){
	return _Projection * _View * _Model;
}

int* GLM::get2DPoint(double x, double y, double z, int width, int height){
	glm::mat4 projectionView = _Projection * _View;
	glm::vec4 point3D = projectionView * glm::vec4(x, y, z, 1);

	int* winPos = new int[2];
	if (point3D[3] > 0){
		winPos[0] = (int)((((point3D[0]/point3D[3]) + 1) / 2.0)*width);
		winPos[1] = (int)(((1 - (point3D[1]/point3D[3])) / 2.0)*height);
	}else{
		winPos[0] = (int)((((point3D[0]/(-point3D[3])) + 1) / 2.0)*width);
		winPos[1] = (int)(((1 - (point3D[1]/(-point3D[3]))) / 2.0)*height);
	}

	return winPos;
}

/*Vector operations*/
glm::vec3 GLM::vectorRotate(double x, double y, double z, float angle, double axisX, double axisY, double axisZ){
        return glm::rotate(glm::vec3(x, y, z), glm::radians(angle), glm::vec3(axisX, axisY, axisZ));	
}
glm::vec3 GLM::normalizeVector(double x, double y, double z){
	return glm::normalize(glm::vec3(x, y, z));
}
glm::vec2 GLM::normalizeVector(double x, double y){
	return glm::normalize(glm::vec2(x, y));
}

glm::vec3 GLM::crossProduct(double x1, double y1, double z1, double x2, double y2, double z2){
	return glm::cross(glm::vec3(x1, y1, z1), glm::vec3(x2, y2, z2));
}
double GLM::dotProduct(double x1, double y1, double z1, double x2, double y2, double z2){
	return glm::dot(glm::vec3(x1, y1, z1), glm::vec3(x2, y2, z2));
}

double GLM::dotProduct(double x1, double y1, double x2, double y2){
	return glm::dot(glm::vec2(x1, y1), glm::vec2(x2, y2));
}

double GLM::vectorLength(double x, double y, double z){
	return glm::length(glm::vec3(x, y, z));
}
