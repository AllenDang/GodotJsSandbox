shader_type spatial;
render_mode wireframe, unshaded, cull_disabled;

// Wireframe debug shader - shows mesh edges
// Uses Godot's built-in wireframe render mode

void fragment() {
    // Bright yellow for high visibility
    ALBEDO = vec3(1.0, 1.0, 0.0);
}
