# Codo
Codo is a simple yet powerful terminal to-do list manager written in C.

## Installation

### Docker

```bash
git clone https://github.com/Korbielowski/codo.git
cd codo
docker build -t codo .
docker run -it -v codo-db-data:/codo/ codo
```

To make your life easier set alias for this command 
```bash
alias codo='docker run -it -v codo-db-data:/codo/ codo'
```

### Ubuntu

```bash
git clone https://github.com/Korbielowski/codo.git
cd codo
make
make install
```

### Dependencies
```bash
apt install sqlite3 libncursesw5-dev
```

## Roadmap

- [X] Task operations
    - [X] Add
    - [X] Delete
    - [X] Edit
    - [X] Change status
- [X] List operations
    - [X] Add
    - [X] Delete
    - [X] Edit
    - [X] Change status
- [ ] Subtasks
- [ ] Details window
- [ ] Color themes
- [ ] Keymaps
- [ ] Config files
- [ ] Search
- [ ] Scalable interface
