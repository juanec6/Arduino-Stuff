import socket
import matplotlib.pyplot as plt
import math

HOST = "127.0.0.1"
PORT = 65432

# --- Polar plot ---
fig, ax = plt.subplots(subplot_kw={'projection': 'polar'})
ax.set_title("Radar (0° – 180°)")

ax.set_theta_zero_location("W")   # 0° a la izquierda (tipo radar frontal)
ax.set_theta_direction(-1)        # sentido horario

ax.set_thetamin(0)
ax.set_thetamax(180)

ax.set_rlabel_position(90)
ax.set_rmax(200)             # ajusta a tu sensor
ax.set_rlim(0, 200)          # rango fijo
ax.set_autoscale_on(False) 

lines = []  # Lista para almacenar todas las líneas dibujadas
last_theta = None
going_forward = True  # True: 0°->180°, False: 180°->0°

# --- TCP server ---
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind((HOST, PORT))
    s.listen()
    print("Server listening...")

    conn, addr = s.accept()
    with conn:
        print("Connected by", addr)

        buffer = ""

        while True:
            data = conn.recv(1024)
            if not data:
                break

            buffer += data.decode(errors="ignore")

            while "\n" in buffer:
                line_str, buffer = buffer.split("\n", 1)

                try:
                    dist_str, theta_str = line_str.split(",")
                    distance = float(dist_str)
                    theta_deg = float(theta_str)
                except ValueError:
                    continue

                theta_rad = math.radians(theta_deg)

                # Detectar cambio de dirección del barrido
                if last_theta is not None:
                    # Detectar cuando llega a 180° (yendo hacia adelante)
                    if going_forward and last_theta > math.radians(170) and theta_deg >= 175:
                        # Limpiar el barrido anterior
                        for line in lines:
                            line.remove()
                        lines = []
                        going_forward = False  # Ahora va de 180° a 0°
                       
                    
                    # Detectar cuando llega a 0° (yendo hacia atrás)
                    elif not going_forward and last_theta < math.radians(10) and theta_deg <= 5:
                        # Limpiar el barrido anterior
                        for line in lines:
                            line.remove()
                        lines = []
                        going_forward = True  # Ahora va de 0° a 180°
                        
                
                last_theta = theta_rad

                # Determinar color según la distancia
                if distance < 60:
                    color = 'r'  # Rojo si la distancia es menor a 60
                    linewidth = 3
                else:
                    color = 'g'  # Verde si la distancia es mayor o igual a 60
                    linewidth = 2

                # Dibujar una línea desde el centro hasta el punto detectado
                line, = ax.plot([theta_rad, theta_rad], [0, distance], color=color, lw=linewidth)
                lines.append(line)

                plt.pause(0.001)

print("Connection closed")
plt.show()