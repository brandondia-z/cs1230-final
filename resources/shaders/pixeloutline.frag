#version 330 core
// uv coord in variable
in vec2 uv_coord;

// add a sampler2D uniform
uniform sampler2D m_texture;

uniform bool isOutline;

uniform int image_width;
uniform int image_height;

out vec4 fragColor;

void make_kernel(inout vec4 n[9], sampler2D tex, vec2 coord){
    float w = 1.0 / image_width;
    float h = 1.0 / image_height;

    n[0] = texture(tex, coord + vec2( -w, -h));
    n[1] = texture(tex, coord + vec2(0.0, -h));
    n[2] = texture(tex, coord + vec2(  w, -h));
    n[3] = texture(tex, coord + vec2( -w, 0.0));
    n[4] = texture(tex, coord);
    n[5] = texture(tex, coord + vec2(  w, 0.0));
    n[6] = texture(tex, coord + vec2( -w, h));
    n[7] = texture(tex, coord + vec2(0.0, h));
    n[8] = texture(tex, coord + vec2(  w, h));
}

void main(){
    // set fragColor using the sampler2D at the UV coordinate
    float width_offset = 1.f/image_width;
    float height_offset = 1.f/image_height;
    float total_r_x, total_g_x, total_b_x;
    float total_r_y, total_g_y, total_b_y;

    fragColor = texture(m_texture, uv_coord); // sets frag color at each uv point to original picture rgb

    vec2 texelsPerPixel = vec2(float(256.f), float (256.f));
    vec2 uv = uv_coord  * texelsPerPixel;

    vec2 alpha = vec2(.05);
    vec2 x = fract(uv);
    vec2 x_ = clamp(0.5f/alpha * x, 0.f, .5f) +
            clamp(0.5f/alpha * (x-1.f) + .5f, 0.f, .5f);

    vec2 textCoord = (floor(uv) + x_) / texelsPerPixel;

    fragColor = texture(m_texture, textCoord);
}

