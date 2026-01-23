# Contributing Guidelines

Thank you for your interest in contributing to this project.

This repository hosts a high-performance 3D physics engine written in modern C++, with a strong focus on numerical accuracy, clean architecture, and scientific correctness.

Contributions are welcome, provided they follow the guidelines below.

---

## License and Contributions

This project is licensed under the **GNU Affero General Public License v3.0 (AGPL-3.0)**.

By contributing to this repository, you agree that:
- your contribution is licensed under AGPL-3.0
- you grant the project maintainer the right to relicense your contribution under a different license, including a commercial license

This is necessary to allow long-term sustainability and potential commercial development of the project.

---

## Contribution Process

1. Fork the repository
2. Create a feature branch
3. Ensure your code:
   - follows the existing architecture
   - is well documented
   - does not break existing tests
4. Submit a pull request to the `main` branch

All contributions are reviewed manually.

---

## Coding Guidelines

- C++23 standard
- Prefer explicit types and clear ownership semantics
- Avoid premature micro-optimisations
- Numerical correctness and clarity take precedence over clever tricks
- Performance-critical code should be justified and documented

---

## Testing

- New features should include appropriate tests
- Existing tests must pass before a pull request can be merged
- Use `ctest` to run the test suite

---

## Code of Conduct

Be respectful and constructive.
This project follows a research-oriented and engineering-focused mindset.
