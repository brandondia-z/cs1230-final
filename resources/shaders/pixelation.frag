#version 330 core

// uv coord in variable
in vec2 uv_coord;

// add a sampler2D uniform
uniform sampler2D m_texture;
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



void main(){       // set fragColor using the sampler2D at the UV coordinate
    float width_offset = 1.f/image_width;
    float height_offset = 1.f/image_height;
    float total_r_x, total_g_x, total_b_x;
    float total_r_y, total_g_y, total_b_y;

    fragColor = texture(m_texture, uv_coord); // sets frag color at each uv point to original picture rgb

//        float grayPixel = (.299*fragColor[0]) + (.587*fragColor[1]) + (.114*fragColor[2]);
//        fragColor = vec4(grayPixel, grayPixel, grayPixel, 1.0);



         vec4 n[9];
        make_kernel(n, m_texture, uv_coord);

        vec4 sobel_edge_h = n[2] + (2.0*n[5]) + n[8] - (n[0] + (2.0*n[3]) + n[6]);
        vec4 sobel_edge_v = n[0] + (2.0*n[1]) + n[2] - (n[6] + (2.0*n[7]) + n[8]);
        vec4 sobel = sqrt((sobel_edge_h * sobel_edge_h) + (sobel_edge_v * sobel_edge_v));

        vec4 outline = vec4(sobel.rgb, 1.0 );
        vec4 outlineColor = vec4(1.0);

        fragColor = vec4(mix(fragColor, outlineColor, outline));

}

