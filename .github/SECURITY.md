# Security Policy

## Supported Versions

| Version | Supported |
|---|---|
| Latest `main` | ✅ |

## Reporting a Vulnerability

Please **do not** open a public GitHub issue for security vulnerabilities, as this exposes the problem before a fix is available.

### Preferred method

Use GitHub's private vulnerability reporting:

1. Go to the [Security tab](https://github.com/TomLaclavere/3DPhysicsEngine/security)
2. Click **"Report a vulnerability"**
3. Fill in the description, impact, and any reproduction steps

### Alternative

Email: **tomlaclavere@gmail.com**  
Subject line: `[SECURITY] 3DPhysicsEngine — <short description>`

## What to include

- A clear description of the vulnerability
- Steps to reproduce
- Potential impact
- Your suggested fix (if any)

## Response timeline

| Stage | Target time |
|---|---|
| Acknowledgement | Within 7 days |
| Status update | Within 14 days |
| Fix or decision | Within 90 days |

## Scope

This project is a local physics simulation engine with no network interface, server component, or user authentication. The attack surface is limited to:

- Malicious input files (YAML config, CSV)
- Build system or dependency vulnerabilities

Out of scope: theoretical issues with no practical exploit path.
