
# cppkg

A small attempt to standardize C++ and modernize application development on it

## Installation
1. clone repo
```
https://github.com/quick-jke/cppkg
```
2. build project
```
cd cppkg
mkdir build && cd build
cmake ..
make
```
3. install
```
sudo make install
```




## API Reference

#### Initialize application

```bash
  cppkg init ${name} --cpp ${cpp_version}
```

| Parameter | Type     | Description                |
| :-------- | :------- | :------------------------- |
| `name` | `string` | Application name |
| `cpp_version` | `int` | **Required**. Cpp version. Supported versions: 11, 14, 17, 20, 23 |

#### Build

```bash
  cppkg build
```


#### Run application

```bash
  cppkg run
```


#### Add library

```bash
  cppkg add ${lib_name}${:lib_version}
```

| Parameter | Type     | Description                |
| :-------- | :------- | :------------------------- |
| `lib_name` | `string` | **Required**. Library name |
| `lib_version` | `string` | Library version |
