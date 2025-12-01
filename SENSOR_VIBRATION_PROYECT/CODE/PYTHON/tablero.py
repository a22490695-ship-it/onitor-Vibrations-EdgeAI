import customtkinter as ctk
import serial
import serial.tools.list_ports
import threading
import time

# --- CONFIGURACIÓN ---
BAUD_RATE = 115200 

class DashboardApp(ctk.CTk):
    def __init__(self):
        super().__init__()

        # Configuración de la ventana
        self.title("Sistema de Mantenimiento Predictivo - ESP32")
        self.geometry("1000x750")
        ctk.set_appearance_mode("Dark")
        ctk.set_default_color_theme("dark-blue")

        self.serial_port = None
        self.is_running = False
        self.current_status = "DESCONECTADO"

        self.setup_ui()
        self.check_ports()

    def setup_ui(self):
        # === BARRA LATERAL ===
        self.sidebar = ctk.CTkFrame(self, width=250, corner_radius=0)
        self.sidebar.pack(side="left", fill="y")

        self.logo_label = ctk.CTkLabel(self.sidebar, text="MONITOR\nVIBRACIONES", font=ctk.CTkFont(size=24, weight="bold"))
        self.logo_label.pack(padx=20, pady=(40, 20))

        self.port_label = ctk.CTkLabel(self.sidebar, text="Seleccionar Puerto COM:", anchor="w")
        self.port_label.pack(padx=20, pady=(10, 0), anchor="w")

        self.port_option = ctk.CTkOptionMenu(self.sidebar, values=["Buscando..."])
        self.port_option.pack(padx=20, pady=10)

        self.btn_connect = ctk.CTkButton(self.sidebar, text="CONECTAR", command=self.toggle_connection, fg_color="#2CC985", hover_color="#25A86F")
        self.btn_connect.pack(padx=20, pady=20)

        self.console_log = ctk.CTkTextbox(self.sidebar, height=200)
        self.console_log.pack(padx=20, pady=20, fill="x")
        self.console_log.insert("0.0", "Sistema listo...\n")

        # === PANEL PRINCIPAL ===
        self.main_frame = ctk.CTkFrame(self, corner_radius=0, fg_color="transparent")
        self.main_frame.pack(side="right", fill="both", expand=True, padx=20, pady=20)

        # Indicador Gigante
        self.status_frame = ctk.CTkFrame(self.main_frame, fg_color="#333333", corner_radius=20)
        self.status_frame.pack(fill="x", pady=20, ipady=20)

        self.lbl_status_title = ctk.CTkLabel(self.status_frame, text="ESTADO DEL MOTOR", font=ctk.CTkFont(size=16))
        self.lbl_status_title.pack(pady=(20, 5))

        self.lbl_main_status = ctk.CTkLabel(self.status_frame, text="ESPERANDO...", font=ctk.CTkFont(size=50, weight="bold"))
        self.lbl_main_status.pack(pady=10)

        self.lbl_sub_status = ctk.CTkLabel(self.status_frame, text="Conecte el dispositivo para iniciar", font=ctk.CTkFont(size=20))
        self.lbl_sub_status.pack(pady=(0, 20))

        # Barras
        self.bars_frame = ctk.CTkFrame(self.main_frame, fg_color="transparent")
        self.bars_frame.pack(fill="both", expand=True, pady=10)

        self.progress_bars = {}
        # Lista visual de etiquetas (sin BLOQUEO)
        self.labels = ["NORMAL", "ROTO", "OBSTRUCCION", "NADA"]
        self.colors = {
            "NORMAL": "#2CC985",
            "ROTO": "#FF4444",
            "OBSTRUCCION": "#FFD700",
            "NADA": "#777777" # Gris
        }

        for i, label in enumerate(self.labels):
            row = ctk.CTkFrame(self.bars_frame, fg_color="transparent")
            row.pack(fill="x", pady=10)

            display_text = label
            if label == "NADA": display_text = "APAGADO"
            
            lbl = ctk.CTkLabel(row, text=display_text, width=120, anchor="e", font=ctk.CTkFont(size=14, weight="bold"))
            lbl.pack(side="left", padx=(0, 20))
            
            progress = ctk.CTkProgressBar(row, height=20)
            progress.set(0)
            progress.pack(side="left", fill="x", expand=True, padx=(0, 20))
            progress.configure(progress_color=self.colors.get(label, "blue"))
            
            self.progress_bars[label] = progress

    def log(self, message):
        self.console_log.insert("end", message + "\n")
        self.console_log.see("end")

    def check_ports(self):
        ports = [port.device for port in serial.tools.list_ports.comports()]
        if ports:
            self.port_option.configure(values=ports)
            if self.port_option.get() not in ports:
                self.port_option.set(ports[0])
        else:
            self.port_option.configure(values=["Sin puertos"])
        self.after(2000, self.check_ports)

    def toggle_connection(self):
        if not self.is_running:
            port = self.port_option.get()
            if port == "Sin puertos" or port == "Buscando...":
                return
            try:
                self.serial_port = serial.Serial(port, BAUD_RATE, timeout=1)
                self.is_running = True
                self.btn_connect.configure(text="DESCONECTAR", fg_color="#FF4444", hover_color="#AA3333")
                self.log(f"Conectado a {port}")
                threading.Thread(target=self.read_serial, daemon=True).start()
            except Exception as e:
                self.log(f"Error conexión: {e}")
        else:
            self.is_running = False
            if self.serial_port:
                self.serial_port.close()
            self.btn_connect.configure(text="CONECTAR", fg_color="#2CC985", hover_color="#25A86F")
            self.log("Desconectado")
            self.update_ui_status("DESCONECTADO")

    def read_serial(self):
        while self.is_running:
            try:
                if self.serial_port.in_waiting > 0:
                    raw_line = self.serial_port.readline().decode('utf-8', errors='ignore').strip()
                    if not raw_line: continue

                    if raw_line.startswith("DATA"):
                        parts = raw_line.split("|")[1:]
                        
                        max_val = -1
                        max_label = "DESCONOCIDO"

                        for part in parts:
                            if ":" in part:
                                label, val_str = part.split(":")
                                try:
                                    val = float(val_str)
                                    
                                    # --- LÓGICA DE FILTRADO ---
                                    
                                    # 1. Convertir BLOQUEO a NADA (Apagado)
                                    if label == "BLOQUEO":
                                        if val > 0.5:
                                            label = "NADA"
                                            val = 1.0
                                    
                                    # 2. Ignorar TIEMPO visualmente
                                    if label == "TIEMPO":
                                        continue  # <--- ESTA LÍNEA DEBE ESTAR INDENTADA

                                    # 3. Actualizar barras
                                    if label in self.progress_bars:
                                        self.progress_bars[label].set(val)
                                    
                                    # 4. Calcular máximo
                                    if val > max_val:
                                        max_val = val
                                        max_label = label
                                except:
                                    pass
                        
                        self.update_ui_status(max_label, max_val)
                    
                    elif "! FALLA" in raw_line:
                         self.log("ALERTA: Falla detectada - Motor detenido")

            except Exception as e:
                print(f"Error hilo: {e}")
                time.sleep(1)

    def update_ui_status(self, label, confidence=0):
        color = "#FFFFFF"
        text = label
        subtext = ""

        if label == "ROTO":
            text = "¡PELIGRO: FALLA CRÍTICA!"
            subtext = "Detectada vibración destructiva"
            color = "#FF4444" 
        
        elif label == "OBSTRUCCION":
            text = "ALERTA: OBSTRUCCIÓN"
            subtext = "El motor está trabajando forzado"
            color = "#FFD700"
        
        elif label == "NORMAL":
            text = "OPERATIVO"
            subtext = "Funcionamiento óptimo"
            color = "#2CC985"
        
        elif label == "NADA":
            text = "MOTOR APAGADO"
            subtext = "Sin actividad detectada"
            color = "#999999" # Gris
        
        elif label == "DESCONECTADO":
            text = "DESCONECTADO"
            subtext = "Seleccione puerto COM"
            color = "#555555"
        
        elif label == "ERROR SENSOR":
            text = "ERROR DE SENSOR"
            subtext = "Verificar cableado I2C"
            color = "#FF8800"

        try:
            self.lbl_main_status.configure(text=text, text_color=color)
            self.lbl_sub_status.configure(text=subtext)
        except:
            pass

if __name__ == "__main__":
    app = DashboardApp()
    app.mainloop()