#version 330 core

out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D texture_diffuse1;
uniform bool hasDiffuseTexture;
uniform bool renderOutline;

uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 viewPos;
uniform vec3 outlineColor;

void main()
{
    if (renderOutline)
    {
        FragColor = vec4(outlineColor, 1.0);
        return;
    }

    vec3 baseColor = hasDiffuseTexture ? texture(texture_diffuse1, TexCoord).rgb : vec3(0.8, 0.8, 0.8);

    vec3 norm = normalize(Normal);
    vec3 L = normalize(-lightDir);
    float diff = max(dot(norm, L), 0.0);

    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-L, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);

    vec3 ambient = 0.25 * baseColor;
    vec3 diffuse = diff * baseColor * lightColor;
    vec3 specular = spec * 0.25 * lightColor;

    vec3 result = ambient + diffuse + specular;

    FragColor = vec4(result, 1.0);
}
