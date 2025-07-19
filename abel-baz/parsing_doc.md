# ✅ Person B – Config & Routing System

This module parses the config file and maps incoming requests to the correct server, location, and resource.

---

## 🔧 What is Implemented

- [x] Tokenizer (config file to tokens)
- [x] Parser (tokens to Config structure)
- [x] Supports multiple `server` blocks
- [x] Multiple `listen` & `server_name` support
- [x] Full location matching logic (prefix-based)
- [x] Redirection support (302-style)
- [x] Index file handling (index.html fallback)
- [x] Autoindex support
- [x] client_max_body_size parsed
- [x] error_page config parsed
- [x] Enforces allowed HTTP methods (GET, POST, DELETE)
- [x] RoutingResult structure for responder use
- [x] Validates port ranges, file existence, permissions

---

## 🔗 Routing Interface (for Person A/C)

Use this after parsing config:

```c
RoutingResult result = routingResult(config, host, port, uri, method);
