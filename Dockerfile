FROM python:3.12-slim

WORKDIR /app

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        build-essential cmake libpython3-dev && \
    rm -rf /var/lib/apt/lists/*

# install dependencies
COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

COPY . .

RUN python setup.py build_ext --inplace

EXPOSE 8000

ENTRYPOINT ["uvicorn"]
CMD ["api.app:app", "--host", "0.0.0.0", "--port", "8000"]