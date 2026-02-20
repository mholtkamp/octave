FROM python:3.12-slim

ARG DOXYBOOK2_VERSION=v1.5.0

RUN apt-get update && apt-get install -y --no-install-recommends \
    doxygen \
    curl \
    unzip \
    ca-certificates \
 && rm -rf /var/lib/apt/lists/*

# Install MkDocs + Material
RUN pip install --no-cache-dir -U pip \
 && pip install --no-cache-dir mkdocs mkdocs-material

# Install doxybook2 (precompiled)
RUN curl -L -o /tmp/doxybook2.zip \
      "https://github.com/matusnovak/doxybook2/releases/download/${DOXYBOOK2_VERSION}/doxybook2-linux-amd64-${DOXYBOOK2_VERSION}.zip" \
 && unzip /tmp/doxybook2.zip -d /tmp/doxybook2 \
 && chmod +x /tmp/doxybook2/bin/doxybook2 \
 && mv /tmp/doxybook2/bin/doxybook2 /usr/local/bin/doxybook2 \
 && rm -rf /tmp/doxybook2 /tmp/doxybook2.zip

WORKDIR /repo
RUN pip install --no-cache-dir -U pip \
 && pip install --no-cache-dir mkdocs mkdocs-material mkdocs-include-markdown-plugin

COPY entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

EXPOSE 8000
ENTRYPOINT ["/entrypoint.sh"]
