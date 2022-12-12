#version 330 core

// uv coord in variable
in vec2 uv_coord;

// add a sampler2D uniform
uniform sampler2D m_texture;


out vec4 fragColor;



void main(){       // set fragColor using the sampler2D at the UV coordinate
    fragColor = texture(m_texture, uv_coord); // sets frag color at each uv point to original picture rgb

    vec2 texelsPerPixel = vec2(float(256.f), float (256.f));
    vec2 uv = uv_coord  * texelsPerPixel;

    vec2 alpha = vec2(.05);
    vec2 x = fract(uv);
    vec2 x_ = clamp(0.5f/alpha * x, 0.f, .5f) +
            clamp(0.5f/alpha * (x-1.f) + .5f, 0.f, .5f);

    vec2 textCoord = (floor(uv) + x_) / texelsPerPixel;

    fragColor =/* vec4(1.f);*/
            texture(m_texture, textCoord);

//            float intensity = (0.299 * fragColor.x) + (0.587 * fragColor.y) + (0.114 * fragColor.z);
//            fragColor = vec4(intensity, intensity, intensity, 1);

}

