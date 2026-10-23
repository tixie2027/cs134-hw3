import struct

def write_long(disk, value):
    disk.write(value.to_bytes(8, "little", signed=True))

def read_long(disk):
    return int.from_bytes(disk.read(8), "little", signed=True)

with open("sys/empty-fs.iso", "r+b") as disk:

    ### svfs_super struct ###
    disk.seek(0)    # svs_magic = 134
    write_long(disk, 134)
    disk.seek(8)    # svs_free_inode = 1
    write_long(disk, 1)
    disk.seek(16)    # svs_nfree = 123
    write_long(disk, 123)
    disk.seek(264)     # svs_root_inode = 0
    write_long(disk, 0)
    disk.seek(272)     # svs_num_inodes = 32
    write_long(disk, 32)
    disk.seek(280)     # svs_version = 1 
    write_long(disk, 1)

    # superblock: available pages 34–62
    # svs_free_blocks[30] 
    # we write in superblock, 24 -> 263
    disk.seek(24)
    for block in range(34, 64):
        write_long(disk, block)

    # last number is the next-page link
    for page in [63, 93, 123]:
        disk.seek(page * 4096)
        for block in range(page + 1, page + 31):
            write_long(disk, block)

    # final continuation page
    disk.seek(153 * 4096)
    for block in range(154, 161):
        write_long(disk, block)

    # 22 empty slots and 1 end-of-list marker
    for slot in range(23):
        write_long(disk, -1)

    # This way, 63 writes 64-93, 93 writes 94-123, etc


    ### writing out all the inodes, page 1 - 32
    for inode in range(1, 32):
        next_inode = inode + 1

        if inode == 31:
            next_inode = -1

        disk.seek((inode + 1) * 4096)
        write_long(disk, next_inode)

    for inode in range(32):
        disk.seek((inode + 1) * 4096 + 8)
        write_long(disk, inode)

    disk.seek(4096 + 24)
    write_long(disk, 33)


    # Root directory’s contents: . and ..
    # This is page 33!

    # current directory: "."
    disk.seek(33 * 4096)
    write_long(disk, 0)  # d_ino
    write_long(disk, 48) # d_off
    disk.write(b".\0")   # d_name

    # parent directory: ".."
    disk.seek(33 * 4096 + 48)
    write_long(disk, 0)
    write_long(disk, 96)
    disk.write(b"..\0")

    # root inode: svi_size
    disk.seek(4096)
    write_long(disk, 96)


    ### CHECKS ###
    # root inode points to page 33
    disk.seek(4096 + 24)
    assert read_long(disk) == 33

    # root directory names
    disk.seek(33 * 4096 + 16)
    assert disk.read(2) == b".\0"

    disk.seek(33 * 4096 + 48 + 16)
    assert disk.read(3) == b"..\0"

    # free inodes 1–30 point to the next inode
    for inode in range(1, 31):
        disk.seek((inode + 1) * 4096)
        assert read_long(disk) == inode + 1

    # inode 31 ends the list
    disk.seek(32 * 4096)
    assert read_long(disk) == -1

    # superblock's free-block numbers
    disk.seek(24)
    for block in range(34, 64):
        assert read_long(disk) == block

    # continuation pages, including their links
    for page in [63, 93, 123]:
        disk.seek(page * 4096)
        for block in range(page + 1, page + 31):
            assert read_long(disk) == block

    # final continuation page
    disk.seek(153 * 4096)
    for block in range(154, 161):
        assert read_long(disk) == block

    for slot in range(23):
        assert read_long(disk) == -1

    print("All checks passed!")