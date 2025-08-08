from fastapi import FastAPI, HTTPException
from pydantic import BaseModel
import mysql.connector
from mysql.connector import Error
import openai
import json
import logging

# =======================
# CONFIGURACIÓN DE LOG
# =======================

logging.basicConfig(
    filename="errores_api.log",
    level=logging.ERROR,
    format="%(asctime)s [%(levelname)s] %(message)s",
    encoding="utf-8"
)

# =======================
# INICIALIZACIÓN FASTAPI
# =======================

app = FastAPI()

# =======================
# CONEXIÓN A OPENAI
# =======================

client = openai.OpenAI(
    api_key=No la pongo por seguridad
)

# =======================
# MODELOS DE DATOS
# =======================

class TextoEntrada(BaseModel):
    descripcion: str

class Tarea(BaseModel):
    tarea: str
    categoria: str
    orden: int

class Credenciales(BaseModel):
    correo: str
    contrasena: str

# =======================
# CONEXIÓN A BASE DE DATOS
# =======================

def conectar_db():
    return mysql.connector.connect(
        host="db",
        user="admin_db",
        password="admin1234",
        database="login_db"
    )

# =======================
# FUNCIONES AUXILIARES
# =======================

def registrar_log(prompt: str, respuesta: str, endpoint: str, status: int):
    try:
        conexion = conectar_db()
        cursor = conexion.cursor()
        consulta = """
            INSERT INTO openai_logs (prompt, respuesta, endpoint, status)
            VALUES (%s, %s, %s, %s)
        """
        datos = (prompt, respuesta, endpoint, status)
        cursor.execute(consulta, datos)
        conexion.commit()
        cursor.close()
        conexion.close()
    except Error as e:
        logging.error(f"[LOG_DB] {str(e)}")

# =======================
# ENDPOINTS
# =======================

@app.get("/")
def leer_root():
    return {"mensaje": "Bienvenido al Planificador de Tareas Técnicas"}

@app.post("/procesar_texto", response_model=list[Tarea])
def procesar_texto(entrada: TextoEntrada):
    try:
        respuesta = client.chat.completions.create(
            model="gpt-3.5-turbo",
            messages=[
                {
                    "role": "system",
                    "content": (
                        "Actuá como un asistente que recibe descripciones de tareas técnicas. "
                        "Extraé las tareas individuales del texto que se te envíe. "
                        "Clasificá cada una en: frontend, backend, base de datos, testing u otro. "
                        "Ordenalas cronológicamente si hay dependencias. "
                        "Respondé estrictamente en formato JSON así:\n"
                        "["
                        '{"tarea": "nombre", "categoria": "frontend/backend/base de datos/testing/otro", "orden": 1}, ...]'
                    )
                },
                {
                    "role": "user",
                    "content": entrada.descripcion
                }
            ],
            temperature=0
        )

        texto_respuesta = respuesta.choices[0].message.content
        tareas = json.loads(texto_respuesta)

        registrar_log(
            prompt=entrada.descripcion,
            respuesta=texto_respuesta,
            endpoint="/procesar_texto",
            status=200
        )

        return tareas

    except Exception as e:
        registrar_log(
            prompt=entrada.descripcion,
            respuesta=str(e),
            endpoint="/procesar_texto",
            status=500
        )
        logging.error(f"[GPT] {str(e)}")
        raise HTTPException(status_code=500, detail="Error al procesar el texto")

@app.post("/login")
def login(cred: Credenciales):
    try:
        conexion = conectar_db()
        cursor = conexion.cursor(dictionary=True)
        query = "SELECT nombre FROM usuarios WHERE correo = %s AND contrasena = %s"
        cursor.execute(query, (cred.correo, cred.contrasena))
        usuario = cursor.fetchone()
        cursor.close()
        conexion.close()

        if usuario:
            return {"mensaje": "Login exitoso", "nombre": usuario["nombre"]}
        else:
            raise HTTPException(status_code=401, detail="Correo o contraseña incorrectos")

    except Exception as e:
        logging.error(f"[LOGIN] {str(e)}")
        raise HTTPException(status_code=500, detail="Error interno del servidor")

@app.get("/historial_logs")
def historial_logs():
    try:
        conexion = conectar_db()
        cursor = conexion.cursor(dictionary=True)
        cursor.execute("SELECT id, prompt, respuesta, timestamp, endpoint, status FROM openai_logs ORDER BY timestamp DESC")
        logs = cursor.fetchall()
        cursor.close()
        conexion.close()
        return logs
    except Exception as e:
        logging.error(f"[HISTORIAL_LOGS] {str(e)}")
        raise HTTPException(status_code=500, detail="No se pudo obtener el historial")

