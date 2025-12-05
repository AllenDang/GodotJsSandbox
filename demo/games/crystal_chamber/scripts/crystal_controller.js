// Magic Crystal Chamber - Shader & Particle Showcase
// Press 1-5 to switch crystal shader effects
// Press Space to trigger particle burst

var time_elapsed = 0;
var current_effect = 0;
var dissolve_direction = 1;
var dissolve_value = 0;

// Effect names for display
var effect_names = [
    "Dissolve Effect",
    "Hologram Effect",
    "Fresnel Glow",
    "Vertex Wobble",
    "Energy Shield"
];

// Reference to child nodes (set in _ready)
var crystal_mesh = null;
var particles_fire = null;
var particles_sparks = null;
var particles_magic = null;
var particles_burst = null;
var floating_orbs = [];

exports._ready = function() {
    console.log("=== Magic Crystal Chamber ===");
    console.log("Press 1-5 to switch shader effects");
    console.log("Press SPACE to trigger particle burst");
    console.log("Current effect: " + effect_names[current_effect]);

    // Get child references
    crystal_mesh = this.get_node("CrystalMesh");
    particles_fire = this.get_node("ParticlesFire");
    particles_sparks = this.get_node("ParticlesSparks");
    particles_magic = this.get_node("ParticlesMagic");
    particles_burst = this.get_node("ParticlesBurst");

    // Get floating orbs
    for (var i = 1; i <= 3; i++) {
        var orb = this.get_node("FloatingOrb" + i);
        if (orb) {
            floating_orbs.push(orb);
        }
    }

    // Start with dissolve effect
    set_effect(0);
};

exports._process = function(delta) {
    time_elapsed += delta;

    // Animate dissolve effect if active
    if (current_effect === 0 && crystal_mesh) {
        dissolve_value += delta * 0.3 * dissolve_direction;
        if (dissolve_value > 1.0) {
            dissolve_value = 1.0;
            dissolve_direction = -1;
        } else if (dissolve_value < 0.0) {
            dissolve_value = 0.0;
            dissolve_direction = 1;
        }

        var material = crystal_mesh.get_surface_override_material(0);
        if (material) {
            material.set_shader_parameter("dissolve_amount", dissolve_value);
        }
    }

    // Rotate floating orbs
    for (var i = 0; i < floating_orbs.length; i++) {
        var orb = floating_orbs[i];
        if (orb) {
            // Orbit around center at different speeds and heights
            var angle = time_elapsed * (0.5 + i * 0.2) + i * 2.094; // 120 degrees apart
            var radius = 2.0 + i * 0.3;
            var height = Math.sin(time_elapsed * (1.0 + i * 0.3)) * 0.5;

            orb.position = {
                x: Math.cos(angle) * radius,
                y: 1.0 + height,
                z: Math.sin(angle) * radius
            };
        }
    }

    // Check for input
    check_input();
};

function check_input() {
    if (Input.is_action_just_pressed("effect_1")) {
        set_effect(0);
    } else if (Input.is_action_just_pressed("effect_2")) {
        set_effect(1);
    } else if (Input.is_action_just_pressed("effect_3")) {
        set_effect(2);
    } else if (Input.is_action_just_pressed("effect_4")) {
        set_effect(3);
    } else if (Input.is_action_just_pressed("effect_5")) {
        set_effect(4);
    }

    if (Input.is_action_just_pressed("trigger_particles")) {
        trigger_particle_burst();
    }
}

function set_effect(index) {
    current_effect = index;
    console.log("Switched to: " + effect_names[index]);

    if (!crystal_mesh) return;

    // Load and apply the appropriate shader material
    var shader_paths = [
        "res://games/crystal_chamber/shaders/dissolve.gdshader",
        "res://games/crystal_chamber/shaders/hologram.gdshader",
        "res://games/crystal_chamber/shaders/fresnel_glow.gdshader",
        "res://games/crystal_chamber/shaders/vertex_wobble.gdshader",
        "res://games/crystal_chamber/shaders/energy_shield.gdshader"
    ];

    // Create a new ShaderMaterial and load the shader
    var shader = load(shader_paths[index]);
    if (shader) {
        var material = new ShaderMaterial();
        material.shader = shader;
        crystal_mesh.set_surface_override_material(0, material);

        // Reset dissolve value when switching to dissolve effect
        if (index === 0) {
            dissolve_value = 0;
            dissolve_direction = 1;
        }
    }
}

function trigger_particle_burst() {
    console.log("Particle burst triggered!");

    // Restart the burst particle system
    if (particles_burst) {
        particles_burst.restart();
        particles_burst.emitting = true;
    }

    // Also create a flash effect by temporarily boosting other particles
    if (particles_sparks) {
        particles_sparks.amount_ratio = 2.0;
        // Reset after a short time (handled in next frames)
    }
}
