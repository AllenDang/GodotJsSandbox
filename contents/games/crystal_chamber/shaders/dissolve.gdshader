shader_type spatial;
render_mode blend_mix, depth_draw_opaque, cull_back, diffuse_burley, specular_schlick_ggx;

uniform vec4 albedo_color : source_color = vec4(0.5, 0.8, 1.0, 1.0);
uniform vec4 edge_color : source_color = vec4(1.0, 0.5, 0.0, 1.0);
uniform float dissolve_amount : hint_range(0.0, 1.0) = 0.0;
uniform float edge_width : hint_range(0.0, 0.2) = 0.05;
uniform float noise_scale : hint_range(1.0, 50.0) = 10.0;
uniform float metallic : hint_range(0.0, 1.0) = 0.3;
uniform float roughness : hint_range(0.0, 1.0) = 0.2;

// Simple noise function
float hash(vec3 p) {
    p = fract(p * 0.3183099 + 0.1);
    p *= 17.0;
    return fract(p.x * p.y * p.z * (p.x + p.y + p.z));
}

float noise(vec3 x) {
    vec3 i = floor(x);
    vec3 f = fract(x);
    f = f * f * (3.0 - 2.0 * f);

    return mix(mix(mix(hash(i + vec3(0, 0, 0)), hash(i + vec3(1, 0, 0)), f.x),
                   mix(hash(i + vec3(0, 1, 0)), hash(i + vec3(1, 1, 0)), f.x), f.y),
               mix(mix(hash(i + vec3(0, 0, 1)), hash(i + vec3(1, 0, 1)), f.x),
                   mix(hash(i + vec3(0, 1, 1)), hash(i + vec3(1, 1, 1)), f.x), f.y), f.z);
}

void fragment() {
    float n = noise(VERTEX * noise_scale);

    // Dissolve threshold
    float threshold = dissolve_amount;

    // Discard pixels below threshold
    if (n < threshold) {
        discard;
    }

    // Edge glow
    float edge = smoothstep(threshold, threshold + edge_width, n);
    vec3 final_color = mix(edge_color.rgb * 2.0, albedo_color.rgb, edge);

    // Add emission at edge
    float emission_strength = 1.0 - edge;

    ALBEDO = final_color;
    METALLIC = metallic;
    ROUGHNESS = roughness;
    EMISSION = edge_color.rgb * emission_strength * 3.0;
}
