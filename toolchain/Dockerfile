FROM alpine

RUN apk --update add make binutils gcc && \
    apk add grub-bios xorriso qemu-system-x86_64 gdb && \
    rm -rf /var/cache/apk/*

ENV PATH "/opt/toolchain:$PATH"
ENV MITTOS64 "true"
ENV BUILDROOT "/opt/"
WORKDIR /opt
