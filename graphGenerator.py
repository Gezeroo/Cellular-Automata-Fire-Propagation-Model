import pandas as pd
import matplotlib.pyplot as plt

# Lê o CSV
df = pd.read_csv("output.csv")

# Cria o gráfico de linha
plt.plot(df['index'], df['veg1'], label='Vegetação 1')
plt.plot(df['index'], df['veg2'], label='Vegetação 2')
plt.plot(df['index'], df['veg3'], label='Vegetação 3')

# Configurações do gráfico
plt.xlabel('Passos de tempo')
plt.ylabel('Área queimada')
plt.title('Humidade 20%')
plt.ylim(0,1)
#plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig("grafico_vetor.png")