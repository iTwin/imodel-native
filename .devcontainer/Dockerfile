#----------------------------------------------------------------------------------------------
# Copyright (c) 2019 Bentley Systems, Incorporated. All rights reserved.
#----------------------------------------------------------------------------------------------

FROM debian:9

RUN \
    apt-get update \
    && apt-get install -y curl \
    && curl -sL https://deb.nodesource.com/setup_10.x | bash - \
    && apt-get install -y git nodejs libnss3 libxss1 libatk-bridge2.0-0 libgtk-3-0 libasound2 locales mercurial python-pip unzip zip clang-3.8 gdb zsh \
    && python -m pip install lxml xxhash future \
    && npm install -g @microsoft/rush \
    && npm config set @bentley:registry https://npm.bentley.com/npm/npm/ \
    && echo "de_DE ISO-8859-1" >> /etc/locale.gen \
    && echo "en_US ISO-8859-1" >> /etc/locale.gen \
    && echo "en_US.UTF-8 UTF-8" >> /etc/locale.gen \
    && echo "fi_FI ISO-8859-1" >> /etc/locale.gen \
    && locale-gen

ENV \
    LANG=en_US.utf8 \
    SrcRoot=/workspace/src/ \
    OutRoot=/workspace/out/ \
    BuildStrategy=iModelJsNodeAddon.Dev \
    BuildArchitecture=LinuxX64 \
    DEBUG=1 \
    LLVM_DEBUG=1 \
    BMAKE_OPT=-I/workspace/src/bsicommon/PublicSDK/ \
    BMAKE_PYTHON_SCRIPTS_DIR=/workspace/src/bsicommon/build/ \
    BSI=1 \
    SHELL=/usr/bin/zsh

RUN \
    sh -c "$(curl -fsSL https://raw.githubusercontent.com/robbyrussell/oh-my-zsh/master/tools/install.sh)" \
    && git config --system --add oh-my-zsh.hide-dirty 1 \
    && git clone --depth 1 https://github.com/junegunn/fzf.git ~/.fzf \
    && ~/.fzf/install --all \
    && echo "alias bb=python\ $SrcRoot'BentleyBuild/BentleyBuild.py'" >> ~/.zshrc

# You are expected to mount an imodel02 $SrcRoot to /workspace/src when running the container.
WORKDIR /workspace/src/imodel02

CMD ["/usr/bin/zsh"]
