#!/usr/bin/env -S docker build --compress -t pvtmert/esp32-at-ppp -f

FROM debian:stable

RUN apt update
RUN apt install -y \
	git curl build-essential

ARG VERSION="1.22.0-80-g6c4433a"
RUN curl -#L "https://dl.espressif.com/dl/xtensa-esp32-elf-linux64-${VERSION}-5.2.0.tar.gz" \
	| tar --strip=1 -xzC /usr/local

RUN apt install -y \
	libffi-dev libssl-dev libncurses-dev flex bison gperf

RUN apt install -y \
	python \
	python-pip \
	python-yaml \
	python-xlrd \
	python-serial \
	python-future \
	python-pyparsing \
	python-setuptools \
	python-cryptography \
	--no-install-recommends

# RUN apt install -y \
# 	python3 \
# 	python3-pip \
# 	python3-future \
# 	python3-serial \
# 	python3-cryptography \
# 	--no-install-recommends
# RUN ln -s ../../bin/python3 /usr/local/bin/python
# ENV PATH "$PATH:/usr/local/bin"

# RUN apt install -y \
# 	binutils-xtensa-lx106 \
# 	gcc-xtensa-lx106 \
# 	--no-install-recommends

WORKDIR /home
SHELL [ "/bin/bash", "--init-file", "/home/esp-idf/add_path.sh", "-c" ]
COPY ./ ./

RUN git reset --hard
RUN git pull -s ours
RUN git submodule update --init --recursive

ENV IDF_PATH "/home/esp-idf"
RUN cd ./esp-idf \
	&& python -m pip install --user -r requirements.txt

RUN make -j $(nproc) patch
RUN make -j $(nproc) defconfig
RUN patch -p1 -bi config.patch

#ENTRYPOINT [ "/bin/bash", "--init-file", "/home/esp-idf/add_path.sh" ]
CMD mkdir -p ./output/ \
	&& make -j $(nproc) bootloader      2>&1 | tee ./output/step.bootloader.log \
	&& make -j $(nproc) build           2>&1 | tee ./output/step.build.log      \
	&& make -j $(nproc) app             2>&1 | tee ./output/step.app.log        \
	&& make -j $(nproc) all             2>&1 | tee ./output/step.all.log        \
	&& make -j $(nproc) size            2>&1 | tee ./output/step.size.log       \
	&& make -j $(nproc) print_flash_cmd 2>&1 | tee ./output/flash.sh            \
	&& cp -v ./build/ota_data_initial.bin      ./output/0x10000_ota_data_initial.bin \
	&& cp -v ./build/bootloader/bootloader.bin ./output/0x1000_bootloader.bin        \
	&& cp -v ./build/at_customize.bin          ./output/0x20000_at_customize.bin     \
	&& cp -v ./build/phy_init_data.bin         ./output/0xf000_phy_init_data.bin     \
	&& cp -v ./build/esp-at.bin                ./output/0x100000_esp-at.bin          \
	&& cp -v ./build/partitions_at.bin         ./output/0x8000_partitions_at.bin     \
	&& ls -l ./output \
	|| bash
