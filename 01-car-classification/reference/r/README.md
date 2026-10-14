# Archived R reconstruction

These three scripts preserve the earlier R reconstruction of the coursework.
The supported implementation is C++17. CMake, CTest, and the documented run
workflow neither execute these scripts nor require R.

The script bodies are retained for historical comparison; their header paths
have been updated after moving them here. They have not been revalidated as
part of the C++ cleanup. If manually executed from the project root, they read
`data/` and write into `results/`, potentially replacing C++ CSV output.
