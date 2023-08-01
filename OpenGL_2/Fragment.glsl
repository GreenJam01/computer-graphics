#version 330 core


struct Material {
vec3 ambient;
vec3 diffuse;
vec3 specular;
float shininess;
};

struct Light {
vec3 ambient;
vec3 diffuse;
vec3 specular;
};

in vec3 fragPos;
 in vec3 fragNorm;

 out vec4 color;

uniform Material material;

 uniform Light light;
 uniform vec3 lightPos;
 uniform vec3 viewPos;

 uniform Light light1;
 uniform vec3 lightPos1;

 void main() {
 // нормализуем полученный вектор нормали
 vec3 norm = normalize(fragNorm);
 // получаем нормализованный вектор направления на источник света
 vec3 lightDir = normalize(lightPos - fragPos);
 // вычисляем косинус угла между полученными векторами
 float cosTheta = max(dot(norm, lightDir), 0.0);

 float powOfCos; // коэффициент бликовой освещенности
 if (cosTheta > 0.0) { // если cosTheta <= 0, бликовая освещенность равна 0
 // получаем нормализованный вектор направления в точку наблюдения
 vec3 viewDir = normalize(viewPos - fragPos);
 // получаем вектор отраженного света
 vec3 lightDirR = reflect(-lightDir, norm);
 powOfCos = pow(max(dot(viewDir, lightDirR), 0.0), material.shininess);

 }
 else
 powOfCos = 0.0;
  
  //2 light
 // получаем нормализованный вектор направления на источник света
 vec3 lightDir1 = normalize(lightPos1 - fragPos);
 // вычисляем косинус угла между полученными векторами
 float cosTheta1 = max(dot(norm, lightDir1), 0.0);

 float powOfCos1; // коэффициент бликовой освещенности
 if (cosTheta1 > 0.0) { // если cosTheta <= 0, бликовая освещенность равна 0
 // получаем нормализованный вектор направления в точку наблюдения
 vec3 viewDir1 = normalize(viewPos - fragPos);
 // получаем вектор отраженного света
 vec3 lightDirR1 = reflect(-lightDir1, norm);
 powOfCos1 = pow(max(dot(viewDir1, lightDirR1), 0.0), material.shininess);

 }
 else
 powOfCos1 = 0.0;

 vec3 ambient1 = light.ambient * material.ambient;
vec3 diffuse1 = light.diffuse * cosTheta1 * material.diffuse;
vec3 specular1 = light.specular * powOfCos1 * material.specular;

vec3 ambient = light.ambient * material.ambient;
vec3 diffuse = light.diffuse * cosTheta * material.diffuse;
vec3 specular = light.specular * powOfCos * material.specular;


 vec3 result = ambient + diffuse + specular + ambient1 + diffuse1 + specular1;
 color = vec4(result, 1.0);
 }