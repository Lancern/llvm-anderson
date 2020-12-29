# p2a

This project implements the two famous
[points-to analysis](https://en.wikipedia.org/wiki/Pointer_analysis)
algorithms, namely 
[Anderson algorithm](https://en.wikipedia.org/w/index.php?title=Andersen%27s_algorithm&action=edit&redlink=1)
and 
[Steensgaard algorithm](https://en.wikipedia.org/wiki/Steensgaard%27s_algorithm),
on the LLVM compiler platform.

## Build

### Prerequisites

Please ensure you have LLVM installed before build. The recommended LLVM version
is LLVM 10.0.0, but other LLVM versions might work as well (not tested).

If you are using Ubuntu, execute the following command to install required
components before build:

```shell
sudo apt install build-essential cmake llvm-dev
```

If you want to build doxygen documentation, you need to install `doxygen` as well:

```shell
sudo apt install doxygen
```

### Build Binaries

Clone the repository and switch into source tree root:

```shell
git clone https://github.com/Lancern/p2a.git
cd p2a
```

Create a build directory:

```shell
mkdir build
cd build
```

Then you can build `p2a` by the familiar two-step build:

```shell
cmake ..
cmake --build .
```

### Build Doxygen Documents

In order to build doxygen documents, just add the `--target` switch when
invoking `cmake --build .` in the last step of build:

```shell
cmake --build . --target doxygen
```

The doxygen output directory is `docs/doxygen`.

## License

This project is open-sourced under the [MIT license](./LICENSE).
