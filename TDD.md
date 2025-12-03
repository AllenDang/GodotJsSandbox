# GodotJSRuntime - Sandbox

## 技术细化设计文档（TDD v2）

---

## 目录

1. 系统架构
2. 从 GodotJS 提取的模块
3. 新增的沙箱模块
4. ScriptLanguage 集成
5. 关卡持久化
6. 模块详细设计
7. 内存安全机制
8. 线程模型
9. 异常与日志
10. 信号（Signal）机制详细设计
11. API 绑定生成系统
12. TypeScript 支持设计
13. 测试方案
14. 补充设计细节
    - 14.1 错误恢复机制
    - 14.2 多沙箱实例设计
    - 14.3 热重载设计
    - 14.4 序列化边界
    - 14.5 JSSandbox 生命周期

---

# 1. 系统架构

## 整体架构

```
┌─────────────────────────────────────────────────────────────┐
│                    GDScript / Game Logic                     │
├─────────────────────────────────────────────────────────────┤
│                       JSSandbox API                          │
│              (eval, set_global, save_level)                  │
├─────────────────────────────────────────────────────────────┤
│  ┌─────────────────┐  ┌─────────────────────────────────┐   │
│  │   sandbox/      │  │         script/                  │   │
│  │  SafeWrapper    │  │  JSScript + JSScriptInstance    │   │
│  │  ObjectRegistry │  │  JSScriptLanguage               │   │
│  │  SandboxConfig  │  │  (处理 _ready/_process)          │   │
│  │  ExecLimiter    │  │                                  │   │
│  └────────┬────────┘  └────────────┬────────────────────┘   │
│           │                        │                         │
│           ▼                        ▼                         │
│  ┌─────────────────────────────────────────────────────┐    │
│  │                    core/                             │    │
│  │   JSEnvironment + TypeConvert + ObjectBindings      │    │
│  │              (从 GodotJS 提取)                        │    │
│  └─────────────────────────┬───────────────────────────┘    │
│                            │                                 │
│                            ▼                                 │
│  ┌─────────────────────────────────────────────────────┐    │
│  │              impl/quickjs/                           │    │
│  │         QuickJS Runtime + Context                    │    │
│  └─────────────────────────────────────────────────────┘    │
├─────────────────────────────────────────────────────────────┤
│                     Godot Engine API                         │
└─────────────────────────────────────────────────────────────┘
```

## 核心设计原则

1. **JS 在主线程执行**：直接调用 Godot API，无需跨线程队列
2. **所有 API 经过 SafeWrapper**：黑名单过滤 + 对象有效性检查
3. **脚本作为一等公民**：JS 脚本可 attach 到节点，支持 _ready/_process
4. **安全的持久化**：只允许写入 user:// 目录

---

# 2. 从 GodotJS 提取的模块

## 提取清单

| GodotJS 原始文件 | 提取后位置 | 修改说明 |
|-----------------|-----------|---------|
| bridge/jsb_environment | core/js_environment | 保持原样 |
| bridge/jsb_type_convert | core/js_type_convert | 保持原样 |
| bridge/jsb_object_bindings | core/js_object_bindings | 插入 SafeWrapper 调用 |
| bridge/jsb_class_register | core/js_class_register | 保持原样 |
| weaver/jsb_script | script/js_script | 简化，去除编辑器功能 |
| weaver/jsb_script_instance | script/js_script_instance | 保持原样 |
| weaver/jsb_script_language | script/js_script_language | 大幅简化 |
| weaver/jsb_resource_loader | script/js_resource_loader | 添加路径安全检查 |
| impl/quickjs/* | impl/quickjs/* | 保持原样 |

## 不提取的模块

| 模块 | 原因 |
|-----|------|
| weaver-editor/* | 编辑器插件，不需要 |
| bridge/jsb_debugger | 调试器，不需要 |
| bridge/jsb_worker | Web Worker，不需要 |

---

# 3. 新增的沙箱模块

## 模块总览

| 模块 | 文件 | 描述 |
|-----|------|------|
| SafeWrapper | sandbox/safe_wrapper.h/cpp | API 安全包装层 |
| ObjectRegistry | sandbox/object_registry.h/cpp | 对象生命周期管理 |
| SandboxConfig | sandbox/sandbox_config.h/cpp | 黑名单配置 |
| ExecutionLimiter | sandbox/execution_limiter.h/cpp | 执行限制 |
| SceneSaver | persistence/scene_saver.h/cpp | 场景持久化 |
| ScriptSaver | persistence/script_saver.h/cpp | 脚本持久化 |

---

# 4. ScriptLanguage 集成

## 4.1 JSScriptLanguage（精简版）

从 GodotJS 的 `jsb_script_language` 提取，去除编辑器相关功能。

**保留的功能：**
- 注册 .js 文件扩展名
- 创建 JSScript 资源
- 管理 JS Environment

**去除的功能：**
- 语法高亮
- 代码补全
- 调试器集成
- 模板生成（编辑器）

```cpp
class JSScriptLanguage : public ScriptLanguageExtension {
    std::shared_ptr<JSEnvironment> environment_;
    SandboxConfig sandbox_config_;  // 新增：沙箱配置

public:
    // 基本接口
    String get_name() const override { return "JavaScript"; }
    String get_type() const override { return "JSScript"; }
    String get_extension() const override { return "js"; }

    void init() override;
    void finish() override;
    void frame() override;  // 每帧更新

    // 脚本创建
    Script* create_script() const override;
    void get_recognized_extensions(List<String>* p_extensions) const override;

    // 沙箱安全检查（新增）
    bool is_api_allowed(const String& class_name, const String& method);
};
```

## 4.2 JSScript（脚本资源）

代表一个 .js 脚本文件，可被 .tscn 引用。

```cpp
class JSScript : public Script {
    String source_code_;
    StringName module_id_;
    bool loaded_ = false;

    // 脚本类信息（从 JS 模块解析）
    StringName js_class_name_;
    StringName native_class_name_;  // 继承的 Godot 类
    HashMap<StringName, PropertyInfo> properties_;
    HashMap<StringName, MethodInfo> methods_;

public:
    // Script 接口
    bool can_instantiate() const override;
    ScriptInstance* instance_create(Object* p_this) override;
    bool has_method(const StringName& p_method) const override;
    StringName get_instance_base_type() const override;

    // 加载
    Error load_source_code(const String& p_path);
    void load_module();
};
```

## 4.3 JSScriptInstance（脚本实例）

绑定到具体 Godot Object，转发方法调用到 JS。

```cpp
class JSScriptInstance : public ScriptInstance {
    Object* owner_;
    Ref<JSScript> script_;
    uint64_t js_object_id_;  // JS 对象 ID
    std::shared_ptr<JSEnvironment> env_;

public:
    // 方法调用（核心）
    bool has_method(const StringName& p_method) const override;

    Variant callp(const StringName& p_method,
                  const Variant** p_args, int p_argcount,
                  Callable::CallError& r_error) override;

    // 属性访问
    bool set(const StringName& p_name, const Variant& p_value) override;
    bool get(const StringName& p_name, Variant& r_ret) const override;

    // 生命周期
    void notification(int p_notification, bool p_reversed) override;

    // 脚本/所有者访问
    Script* get_script() const override { return script_.ptr(); }
    Object* get_owner() const override { return owner_; }
    ScriptLanguage* get_language() const override;

    // 属性列表（用于编辑器/保存）
    void get_property_list(List<PropertyInfo>* p_properties) const override;
    bool property_can_revert(const StringName& p_name) const override;
    bool property_get_revert(const StringName& p_name, Variant& r_ret) const override;
};
```

**关键：_ready/_process 调用流程**

```
Godot 调用 Node._ready()
    ↓
JSScriptInstance::callp("_ready", ...)
    ↓
env_->call_method(js_object_id_, "_ready", args)
    ↓
QuickJS 执行 JS 对象的 _ready 方法
```

---

# 5. 关卡持久化

## 5.1 场景保存流程

```
sandbox.save_level("user://levels/my_level/")
    ↓
SceneSaver::save(directory)
    ├── 1. 收集所有动态创建的节点
    ├── 2. 收集所有 attach 的 JS 脚本
    ├── 3. 保存脚本文件 (.js)
    ├── 4. 创建 PackedScene
    ├── 5. 保存场景文件 (.tscn)
    └── 6. 保存元数据 (.json)
```

## 5.2 SceneSaver

```cpp
class SceneSaver {
public:
    struct SaveOptions {
        bool include_external_resources = false;  // 是否复制外部资源到目标目录
        bool fail_on_gdscript = true;             // 遇到 GDScript 是否失败
        PackedStringArray allowed_resource_types; // 允许的外部资源类型
    };

    struct SaveResult {
        Error error;
        String scene_path;
        PackedStringArray script_paths;
        PackedStringArray warnings;               // 警告信息（如跳过的节点）
        PackedStringArray copied_resources;       // 复制的外部资源列表
    };

    static SaveResult save(Node* root, const String& directory,
                          const SaveOptions& options = {});

private:
    // 路径安全检查
    static bool validate_path(const String& path) {
        return path.begins_with("user://") && !path.contains("..");
    }

    // 检测循环引用
    static bool detect_circular_reference(Node* root, HashSet<ObjectID>& visited);

    // 收集需要保存的脚本
    static Dictionary collect_scripts(Node* root, PackedStringArray& warnings);

    // 处理外部资源引用（纹理、音频等）
    static Error process_external_resources(Node* node,
                                           const String& dest_dir,
                                           const SaveOptions& options,
                                           Dictionary& resource_map);

    // 处理非 JS 脚本的节点
    static Error handle_non_js_script(Node* node,
                                     const SaveOptions& options,
                                     PackedStringArray& warnings);

    // 保存单个脚本文件
    static Error save_script(const String& source, const String& path);

    // 创建并保存 PackedScene
    static Error save_scene(Node* root, const String& path,
                           const Dictionary& script_paths,
                           const Dictionary& resource_map);
};
```

**边界情况处理：**

| 情况 | 处理策略 |
|-----|---------|
| 循环引用 | 检测并返回错误，不支持保存 |
| 外部资源（.png, .tres） | 可选复制到目标目录，或保持原路径引用 |
| GDScript 节点 | 默认失败，可配置为警告并跳过脚本 |
| 空脚本节点 | 正常保存节点，不生成脚本文件 |
| 无法序列化的属性 | 跳过并记录警告 |

## 5.3 保存的文件结构

```
user://levels/my_level/
├── level.tscn              # 场景文件（标准 Godot 格式）
├── floating_platform.js    # 行为脚本 1
├── coin_pickup.js          # 行为脚本 2
└── level.json              # 元数据
```

**level.tscn 示例：**

```gdscript
[gd_scene load_steps=3 format=3]

[ext_resource type="Script" path="user://levels/my_level/floating_platform.js" id="1"]

[node name="Level" type="Node3D"]

[node name="Platform1" type="StaticBody3D" parent="."]
script = ExtResource("1")
amplitude = 1.5
speed = 0.8
```

**level.json 示例：**

```json
{
    "version": 1,
    "created_at": "2024-01-15T10:30:00Z",
    "sandbox_version": "1.0.0",
    "scripts": [
        "floating_platform.js",
        "coin_pickup.js"
    ]
}
```

---

# 6. 模块详细设计

## 6.1 JSSandbox（对外接口）

```cpp
class JSSandbox : public RefCounted {
    GDCLASS(JSSandbox, RefCounted);

    std::shared_ptr<JSEnvironment> env_;
    Ref<SandboxConfig> config_;
    Ref<ObjectRegistry> registry_;
    Ref<ExecutionLimiter> limiter_;

    // 跟踪动态创建的节点（用于保存）
    Vector<ObjectID> created_nodes_;

protected:
    static void _bind_methods();

public:
    // 配置
    void set_timeout_ms(int ms);
    void set_memory_limit_mb(int mb);
    Error load_blocklist(const String& path);

    // 执行
    Variant eval(const String& code);
    Variant eval_file(const String& path);

    // 全局变量
    void set_global(const String& name, const Variant& value);
    Variant get_global(const String& name);

    // 关卡持久化
    Error save_level(const String& directory);
    Array get_created_nodes();

    // 信号
    // signal error_occurred(message: String, line: int, column: int)
    // signal console_output(message: String)
};
```

## 6.2 SafeWrapper（安全包装）

所有 Godot API 调用的入口点。

```cpp
class SafeWrapper {
    SandboxConfig* config_;
    ObjectRegistry* registry_;
    ExecutionLimiter* limiter_;

public:
    // 方法调用
    Variant call_method(uint64_t handle, const StringName& method,
                       const Variant** args, int argc);

    // 属性访问
    Variant get_property(uint64_t handle, const StringName& property);
    void set_property(uint64_t handle, const StringName& property,
                     const Variant& value);

    // 对象创建
    uint64_t create_object(const StringName& class_name);

private:
    bool check_allowed(const String& class_name, const String& method);
    bool check_object_valid(uint64_t handle);
    bool check_call_limit();
};
```

## 6.3 ObjectRegistry（对象注册）

```cpp
struct Handle {
    ObjectID id;
    uint32_t version;
    bool valid;
    bool is_ref_counted;
};

class ObjectRegistry {
    HashMap<uint64_t, Handle> handles_;
    uint32_t next_version_ = 1;

public:
    uint64_t create_handle(Object* obj);
    Object* get_object(uint64_t handle_key);
    void mark_deleted(ObjectID id);
    void on_js_gc(uint64_t handle_key);

    // 监听对象删除
    void setup_deletion_callback(Object* obj);
};
```

---

## 6.4 SandboxConfig（黑名单配置）

```cpp
class SandboxConfig {
    HashSet<String> blocked_classes_;
    HashMap<String, HashSet<String>> blocked_methods_;

public:
    Error load(const String& path);  // 从 TOML 加载

    bool is_class_blocked(const String& class_name) const;
    bool is_method_blocked(const String& class_name, const String& method) const;
};
```

**配置文件示例 (sandbox.toml)：**

```toml
[blocked_classes]
classes = [
    # 系统访问
    "OS", "FileAccess", "DirAccess",
    # 多线程
    "Thread", "Mutex", "Semaphore", "WorkerThreadPool",
    # 网络
    "TCPServer", "UDPServer", "HTTPClient", "HTTPRequest",
    "WebSocketPeer", "ENetConnection", "ENetMultiplayerPeer",
    # 脚本/代码执行
    "JavaScriptBridge", "Expression", "GDScript", "CSharpScript",
    # 扩展/原生代码
    "NativeExtension", "GDExtensionManager",
    # 资源管理（通过安全的 load() 代替）
    "ResourceLoader", "ResourceSaver"
]

[blocked_methods]
Object = ["call", "callv", "set", "get_script"]
ClassDB = ["instantiate", "instance", "can_instantiate", "get_class_list"]
Engine = ["get_singleton", "register_singleton", "unregister_singleton"]
# 注意：set_script 不阻止，由 SafeWrapper 验证只允许 JSScript
```

## 6.5 资源加载安全（SafeResourceLoader）

```cpp
class SafeResourceLoader {
    // 允许加载的资源扩展名
    static const HashSet<String> ALLOWED_EXTENSIONS;
    // 禁止加载的资源扩展名
    static const HashSet<String> BLOCKED_EXTENSIONS;

public:
    static Variant safe_load(const String& path);

private:
    static bool validate_path(const String& path);
    static bool validate_extension(const String& ext);
};

// 允许的扩展名
const HashSet<String> SafeResourceLoader::ALLOWED_EXTENSIONS = {
    // 场景
    "tscn", "scn",
    // 图片
    "png", "jpg", "jpeg", "webp", "svg",
    // 音频
    "ogg", "wav", "mp3",
    // 3D 模型
    "glb", "gltf",
    // 资源
    "tres", "material",
    // 字体
    "ttf", "otf",
    // 脚本（仅 JS）
    "js"
};

// 禁止的扩展名
const HashSet<String> SafeResourceLoader::BLOCKED_EXTENSIONS = {
    "gd",       // GDScript
    "cs",       // C#
    "gdns",     // NativeScript
    "gdnlib",   // GDNative 库
    "so", "dll", "dylib",  // 原生库
    "gdextension"          // GDExtension
};
```

## 6.6 ExecutionLimiter（执行限制）

```cpp
class ExecutionLimiter {
    int64_t timeout_ms_ = 1000;
    int64_t memory_limit_bytes_ = 64 * 1024 * 1024;
    int call_count_ = 0;
    int call_limit_per_frame_ = 200;
    int64_t deadline_ = 0;

public:
    void set_timeout_ms(int64_t ms);
    void set_memory_limit_mb(int mb);
    void set_call_limit(int limit);

    void begin_execution();  // 设置 deadline
    bool check_timeout();
    bool check_call_limit();
    void reset_frame_counter();

    // QuickJS interrupt handler
    static int interrupt_handler(JSRuntime* rt, void* opaque);
};
```

---

# 7. 内存安全机制

## 核心保证

- JS 永不持有裸指针
- 所有对象通过 handle 访问
- Node 删除监听 NOTIFICATION_PREDELETE
- RefCounted 使用 reference/unreference 同步
- generation/version 防止 ID 复用
- queue_free 自动清理 JS 句柄
- JS GC 时减少 Godot 引用计数

## 对象删除流程

```
Node 被 queue_free()
    ↓
帧结束时真正删除
    ↓
NOTIFICATION_PREDELETE 触发
    ↓
ObjectRegistry::mark_deleted(id)
    ↓
JS 后续访问 → 抛出异常（不崩溃）
```

---

# 8. 线程模型

**核心决策：JS 在主线程执行**

QuickJS 是单线程库，将 JS 放在主线程执行可以：
- 直接调用 Godot API，无需跨线程队列
- 避免复杂的同步机制和死锁风险
- 简化架构，减少 bug

执行模型：

```
Godot Main Thread
    │
    ├── JSSandbox.eval()
    │       │
    │       └── QuickJS 执行 JS
    │               │
    │               └── SafeWrapper 直接调用 Godot API
    │
    └── 继续渲染循环
```

**防止主线程阻塞的保护机制：**

1. **执行超时**：interrupt handler 限制单次 eval 执行时间
2. **指令计数**：防止死循环
3. **调用频率**：限制每帧 API 调用次数

如果将来需要长时间运行的 JS 任务，可以：
- 分帧执行（yield/async 模式）
- 或在独立线程执行纯计算，但所有 Godot API 调用仍需回到主线程

---

# 9. 异常与日志

所有错误使用统一格式：

```
JS Error at <filename>:<line> - <message>
```

设计 signal：

```
signal error_occurred(String message)
signal console_output(String msg)
```

---

# 10. 信号（Signal）机制详细设计

## 10.1 信号连接存储结构

```cpp
struct SignalConnection {
    ObjectID target_id;           // 发射信号的对象
    StringName signal_name;       // 信号名称
    JSValue callback;             // JS 回调函数（必须通过 JS_DupValue 持有）
    uint64_t connection_id;       // 连接唯一 ID
};

class SignalRegistry {
    JSContext* ctx_;  // 用于管理 JSValue 引用计数
    // 按对象 ID 索引的连接列表
    HashMap<ObjectID, Vector<SignalConnection>> connections_by_object_;
    // 按连接 ID 快速查找
    HashMap<uint64_t, SignalConnection*> connections_by_id_;
    uint64_t next_connection_id_ = 1;

public:
    SignalRegistry(JSContext* ctx) : ctx_(ctx) {}

    uint64_t connect(Object* target, const StringName& signal, JSValue callback);
    void disconnect(uint64_t connection_id);
    void disconnect_all(ObjectID target_id);  // 对象删除时调用
    void cleanup_sandbox();  // 沙箱销毁时清理所有连接
};
```

**JSValue 引用计数管理（重要）：**

```cpp
// 连接时：增加引用计数
uint64_t SignalRegistry::connect(Object* target, const StringName& signal, JSValue callback) {
    SignalConnection conn;
    conn.target_id = target->get_instance_id();
    conn.signal_name = signal;
    conn.callback = JS_DupValue(ctx_, callback);  // 必须 DupValue！
    conn.connection_id = next_connection_id_++;
    // ... 存储 conn
    return conn.connection_id;
}

// 断开时：释放引用计数
void SignalRegistry::disconnect(uint64_t id) {
    if (auto* conn = connections_by_id_.getptr(id)) {
        JS_FreeValue(ctx_, conn->callback);  // 必须 FreeValue！
        // ... 移除 conn
    }
}

// 沙箱销毁时：释放所有引用
void SignalRegistry::cleanup_sandbox() {
    for (auto& [obj_id, connections] : connections_by_object_) {
        for (auto& conn : connections) {
            JS_FreeValue(ctx_, conn.callback);
        }
    }
    connections_by_object_.clear();
    connections_by_id_.clear();
}
```

## 10.2 信号连接流程

```
JS: let conn = player.connect("health_changed", callback)
    ↓
SafeWrapper::connect_signal(handle, signal_name, js_callback)
    ↓
1. 验证对象有效性
2. JS_DupValue 增加回调引用计数
3. 创建 Callable 包装 JS 回调
4. 调用 Godot Object::connect()
5. SignalRegistry 记录连接
6. 返回 SignalConnection 对象给 JS
```

**JS 端 SignalConnection 类：**

```javascript
class SignalConnection {
    constructor(connection_id) {
        this._id = connection_id;
        this._connected = true;
    }

    disconnect() {
        if (this._connected) {
            __native_disconnect(this._id);
            this._connected = false;
        }
    }

    is_connected() {
        return this._connected && __native_is_connected(this._id);
    }
}
```

## 10.3 信号断开与自动清理

**手动断开：**
```javascript
let connection = player.connect("health_changed", callback);
// 稍后断开
connection.disconnect();
```

**对象删除时自动清理：**
```
NOTIFICATION_PREDELETE 触发
    ↓
ObjectRegistry::mark_deleted(id)
    ↓
SignalRegistry::disconnect_all(id)
    ↓
释放所有相关 JS 回调引用
```

**沙箱销毁时清理：**
```
JSSandbox 析构
    ↓
SignalRegistry::cleanup_sandbox()
    ↓
断开所有信号连接，释放所有 JS 回调
```

---

# 11. API 绑定生成系统

## 11.1 构建时生成（BUILD TIME）

**不使用运行时 ClassDB 反射**，而是在构建时预生成所有绑定代码。

```
构建流程：
1. 运行 Godot Editor --dump-extension-api
2. 解析 extension_api.json
3. 生成 C++ 绑定代码 + JS 包装器
4. 应用黑名单过滤
5. 输出到 generated/ 目录
```

## 11.2 生成器结构

```
scripts/
├── generate_bindings.py      # 主生成脚本
├── templates/
│   ├── class_binding.cpp.j2  # C++ 绑定模板
│   └── class_wrapper.js.j2   # JS 包装模板
└── config/
    └── blocklist.json        # 黑名单配置

generated/
├── bindings/
│   ├── node_bindings.gen.cpp
│   ├── spatial_bindings.gen.cpp
│   └── ...
├── wrappers/
│   ├── node_wrapper.gen.js
│   └── ...
└── class_registry.gen.cpp    # 类注册表
```

## 11.3 生成的绑定代码示例

```cpp
// generated/bindings/node3d_bindings.gen.cpp
void register_Node3D_bindings(JSContext* ctx, JSValue global) {
    JSValue proto = JS_NewObject(ctx);

    // 属性绑定
    JS_DefinePropertyGetSet(ctx, proto,
        JS_NewAtom(ctx, "position"),
        JS_NewCFunction(ctx, js_Node3D_get_position, "get_position", 0),
        JS_NewCFunction(ctx, js_Node3D_set_position, "set_position", 1),
        JS_PROP_CONFIGURABLE);

    // 方法绑定（经过黑名单过滤）
    JS_SetPropertyStr(ctx, proto, "add_child",
        JS_NewCFunction(ctx, js_Node3D_add_child, "add_child", 1));

    // 注册构造函数
    JSValue ctor = JS_NewCFunction2(ctx, js_Node3D_constructor,
        "Node3D", 0, JS_CFUNC_constructor, 0);
    JS_SetConstructor(ctx, ctor, proto);
    JS_SetPropertyStr(ctx, global, "Node3D", ctor);
}
```

## 11.4 黑名单过滤

生成器在生成时自动跳过黑名单中的类和方法：

```python
# generate_bindings.py
def should_generate_method(class_name, method_name):
    if class_name in BLOCKED_CLASSES:
        return False
    if method_name in BLOCKED_METHODS.get(class_name, []):
        return False
    if method_name in GLOBAL_BLOCKED_METHODS:
        return False
    return True
```

---

# 12. TypeScript 支持设计

## 12.1 编译策略

采用**运行时编译**方案（可选预编译优化）：

```
TypeScript 代码
    ↓
内嵌 TypeScript 编译器（精简版）
    ↓
JavaScript 代码
    ↓
QuickJS 执行
```

## 12.2 TypeScript 编译器集成

```cpp
class TypeScriptCompiler {
    JSRuntime* compiler_rt_;   // 独立的编译器 Runtime
    JSContext* compiler_ctx_;  // 独立的编译器上下文
    JSValue ts_compile_fn_;    // TypeScript transpile 函数

public:
    Error init();  // 加载 TypeScript 编译器
    String compile(const String& ts_source);  // TS -> JS
    void shutdown();
};
```

**编译器加载：**
```cpp
Error TypeScriptCompiler::init() {
    // 创建独立的 Runtime，与沙箱隔离
    compiler_rt_ = JS_NewRuntime();
    // 设置编译器专用内存限制（32MB，仅用于编译）
    JS_SetMemoryLimit(compiler_rt_, 32 * 1024 * 1024);

    compiler_ctx_ = JS_NewContext(compiler_rt_);

    // 加载精简版 TypeScript 编译器
    String compiler_js = load_bundled_resource("res://typescript/compiler.js");
    JS_Eval(compiler_ctx_, compiler_js.utf8().get_data(), ...);
    ts_compile_fn_ = JS_GetPropertyStr(compiler_ctx_, global, "transpile");
    return OK;
}
```

**编译器方案选择：**

| 方案 | 体积 | 性能 | 推荐场景 |
|-----|------|------|---------|
| 方案 A：esbuild-wasm | ~8MB | 快 | 完整功能，开发环境 |
| 方案 B：swc-wasm | ~2MB | 很快 | 轻量运行时编译 |
| 方案 C：预编译模式 | 0 | N/A | 生产环境，最小体积 |

**推荐策略：**
- 默认使用方案 C（预编译），分发体积最小
- 可选启用运行时编译（方案 B），适合开发/Mod 场景
- 编译完成后可调用 `shutdown()` 释放编译器上下文以节省内存

## 12.3 @export 装饰器支持

TypeScript 装饰器转换为 Godot 属性导出：

```typescript
// 输入
export default class Enemy extends CharacterBody3D {
    @export speed: number = 100;
    @export health: number = 100;
}

// 编译后 JS（带元数据）
class Enemy extends CharacterBody3D {
    static __exports__ = {
        speed: { type: "float", default: 100 },
        health: { type: "float", default: 100 }
    };
    speed = 100;
    health = 100;
}
```

## 12.4 类型定义文件生成

与 API 绑定生成同步，自动生成 `.d.ts` 文件：

```
generated/
├── types/
│   ├── godot.d.ts           # 所有 Godot 类型定义
│   ├── math.d.ts            # Vector2, Vector3, Color 等
│   └── index.d.ts           # 入口文件
```

**示例 godot.d.ts：**
```typescript
declare class Node3D extends Node {
    position: Vector3;
    rotation: Vector3;
    scale: Vector3;

    add_child(node: Node, force_readable_name?: boolean): void;
    remove_child(node: Node): void;
    get_children(): Node[];
}
```

---

# 13. 测试方案

## 13.1 沙箱安全测试

| 测试类型       | 示例                                      |
| -------------- | ----------------------------------------- |
| 内存安全       | queue_free 后访问对象应抛出 JS 异常       |
| 生命周期       | Node 删除后 JS 报错，不崩溃               |
| 指令限制       | while(true) 应被 interrupt handler 中断   |
| 黑名单         | 禁止调用 OS、FileAccess 等类              |
| 黑名单方法     | 禁止 Object.call、Object.set_script 等    |
| 资源泄漏       | JS 创建 RefCounted 对象后 GC，C++ 正确 unref |
| 场景树动态操作 | 大量创建/删除节点不崩溃                   |
| 路径安全       | load() 拒绝绝对路径和 ../ 穿越            |
| 信号机制       | connect/disconnect 正确工作，对象删除时自动清理 |

## 13.2 ScriptLanguage 集成测试

| 测试类型       | 示例                                      |
| -------------- | ----------------------------------------- |
| 脚本加载       | .js 文件可被 ResourceLoader 正确加载      |
| 脚本实例化     | JSScript 可 attach 到 Node 并正确实例化   |
| _ready 调用    | 节点进入场景树时 _ready() 被正确调用      |
| _process 调用  | 每帧 _process(delta) 被正确调用           |
| 属性导出       | @export 属性在编辑器/运行时正确暴露       |
| 方法调用       | GDScript 可调用 JS 脚本的方法             |
| 继承验证       | JS 类正确继承指定的 Godot 类型            |

## 13.3 关卡持久化测试

| 测试类型       | 示例                                      |
| -------------- | ----------------------------------------- |
| 场景保存       | save_level() 生成有效的 .tscn 文件        |
| 脚本保存       | JS 脚本文件正确写入 user:// 目录          |
| 路径安全       | 禁止保存到 user:// 以外的目录             |
| 场景加载       | 保存的 .tscn 可被 load() 正确加载         |
| 脚本关联       | 加载后节点的 JS 脚本正确执行              |
| 属性恢复       | 保存的属性值在加载后正确恢复              |
| 元数据         | level.json 包含正确的版本和脚本列表       |

---

# 14. 补充设计细节

## 14.1 错误恢复机制

JS 执行出错后的恢复策略：

**1. 单次 eval 错误**
- 捕获异常，返回错误信息
- JSContext 状态保持不变
- 发射 `error_occurred` 信号

**2. _process 中错误**
```cpp
class JSScriptInstance {
    int consecutive_errors_ = 0;
    static const int MAX_CONSECUTIVE_ERRORS = 3;

    void handle_process_error(const String& error) {
        consecutive_errors_++;
        emit_error(error);

        if (consecutive_errors_ >= MAX_CONSECUTIVE_ERRORS) {
            // 禁用此脚本的 _process 调用
            process_enabled_ = false;
            WARN_PRINT("JS script disabled after consecutive errors");
        }
    }

    void on_process_success() {
        consecutive_errors_ = 0;  // 重置计数
    }
};
```

**3. 严重错误（内存耗尽/死循环中断）**
- 重置 JSContext
- 清理所有 JS 对象引用
- 保留 Godot 节点，但脚本失效
- 发射 `sandbox_reset` 信号通知游戏逻辑

```cpp
void JSSandbox::handle_fatal_error() {
    // 1. 断开所有信号连接
    signal_registry_->cleanup_sandbox();

    // 2. 清理 ObjectRegistry
    object_registry_->clear_all();

    // 3. 重置 JSContext
    JS_FreeContext(ctx_);
    ctx_ = JS_NewContext(runtime_);
    reinitialize_bindings();

    // 4. 通知游戏
    emit_signal("sandbox_reset");
}
```

## 14.2 多沙箱实例设计

采用：**单 JSRuntime + 多 JSContext** 架构

```
JSRuntime (全局唯一，管理内存池)
    │
    ├── JSContext 1 (JSSandbox A - 当前编辑的关卡)
    ├── JSContext 2 (JSSandbox B - 已加载的关卡 1)
    └── JSContext 3 (JSSandbox C - 已加载的关卡 2)
```

**设计要点：**

| 层级 | 共享内容 | 隔离内容 |
|-----|---------|---------|
| JSRuntime | 内存池、Godot 类绑定 | - |
| JSContext | - | 全局变量、脚本实例、信号连接 |

**实现：**

```cpp
class JSRuntimeManager {
    static JSRuntime* shared_runtime_;
    static int context_count_;

public:
    static JSRuntime* get_runtime() {
        if (!shared_runtime_) {
            shared_runtime_ = JS_NewRuntime();
            JS_SetMemoryLimit(shared_runtime_, 128 * 1024 * 1024);  // 总内存限制
            register_godot_bindings(shared_runtime_);
        }
        return shared_runtime_;
    }

    static JSContext* create_context() {
        JSContext* ctx = JS_NewContext(get_runtime());
        context_count_++;
        return ctx;
    }

    static void free_context(JSContext* ctx) {
        JS_FreeContext(ctx);
        context_count_--;
        if (context_count_ == 0) {
            JS_FreeRuntime(shared_runtime_);
            shared_runtime_ = nullptr;
        }
    }
};
```

**优点：**
- 关卡间代码隔离，一个出错不影响其他
- 共享 Godot 类绑定，减少内存开销
- 独立销毁，清理简单

## 14.3 热重载设计

保存后脚本修改的处理：

**开发/调试场景（可选功能）：**

```cpp
class JSScriptWatcher {
    HashMap<String, uint64_t> file_mod_times_;

public:
    void check_for_changes() {
        for (auto& [path, mod_time] : file_mod_times_) {
            uint64_t current = FileAccess::get_modified_time(path);
            if (current > mod_time) {
                reload_script(path);
                file_mod_times_[path] = current;
            }
        }
    }

private:
    void reload_script(const String& path) {
        // 1. 找到使用此脚本的所有节点
        Vector<Node*> nodes = find_nodes_with_script(path);

        // 2. 重新加载脚本资源
        Ref<JSScript> script = ResourceLoader::load(path, "", CACHE_IGNORE);

        // 3. 重新创建脚本实例
        for (Node* node : nodes) {
            node->set_script(script);
            // 这会触发新的 _ready() 调用
        }
    }
};
```

**生产场景：**
- 不支持热重载
- 需要重新加载整个关卡场景
- 这是预期行为，保证状态一致性

## 14.4 序列化边界

**可持久化：**
- 节点树结构
- 节点类型和名称
- `@export` 标记的属性
- 脚本源代码

**不可持久化：**
- JS 闭包状态
- 运行时动态创建的变量（非 @export）
- 信号连接（加载后需重新执行 _ready 建立）
- 临时对象引用

**设计原则：关卡保存的是"初始状态"，不是"运行时快照"**

```javascript
// 示例：可保存 vs 不可保存
class Enemy extends CharacterBody3D {
    // ✓ 可保存（@export 属性）
    static __exports__ = {
        max_health: { type: "int", default: 100 },
        speed: { type: "float", default: 5.0 }
    };
    max_health = 100;
    speed = 5.0;

    // ✗ 不保存（运行时状态）
    current_health = 100;
    target = null;
    path = [];

    _ready() {
        // 加载时重新初始化运行时状态
        this.current_health = this.max_health;
        this.target = null;
        this.path = [];
    }
}
```

## 14.5 JSSandbox 生命周期

```
JSSandbox 创建
    │
    ├── 从 JSRuntimeManager 获取/创建共享 Runtime
    ├── 创建独立 JSContext
    ├── 初始化 ObjectRegistry
    ├── 初始化 SignalRegistry
    └── 初始化 ExecutionLimiter
    │
    ▼
JSSandbox 使用中
    │
    ├── eval() 执行代码
    ├── 节点创建/删除
    ├── 信号连接/断开
    └── 可选：save_level()
    │
    ▼
JSSandbox 销毁
    │
    ├── SignalRegistry::cleanup_sandbox()  // 断开所有信号
    ├── ObjectRegistry::clear_all()        // 清理对象引用
    ├── JS_FreeContext()                   // 释放 Context
    └── JSRuntimeManager::free_context()   // 可能释放 Runtime
```

---
