import pandas as pd
import matplotlib.pyplot as plt

# Lê o CSV
nomes_imgs = ['output_0_.csv', 'output_1_.csv','output_2_.csv','output_3_.csv']
nomes_imgsA = ['outputAlpha_0_.csv', 'outputAlpha_1_.csv','outputAlpha_2_.csv','outputAlpha_3_.csv','outputAlpha_4_.csv']
saidas = ['20','40','60','80']
saidasA = ['0.078','0.24','0.5','0.76','1']

"""
df = pd.read_csv('output_0_.csv')

# Cria o gráfico de linha
plt.plot(df['index'], df['veg2'], label='Floresta')
plt.plot(df['index'], df['veg1'], label='Savana')
plt.plot(df['index'], df['veg3'], label='Campo')

# Configurações do gráfico
plt.xlabel('Passos de tempo')
plt.ylabel('Área queimada')
plt.title('umidade 40%')
plt.ylim(0,1)
#plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig("umidade40.png")
plt.clf()
"""

df = pd.read_csv('outputAlpha_0_.csv')
plt.plot(df['index'], df['Aveg2L'], label='Floresta - Subida')
plt.plot(df['index'], df['Aveg2R'], label='Floresta - Descida')
plt.plot(df['index'], df['Aveg1L'], label='Savana - Subida')
plt.plot(df['index'], df['Aveg1R'], label='Savana - Descida')
plt.plot(df['index'], df['Aveg3L'], label='Campo - Subida')
plt.plot(df['index'], df['Aveg3R'], label='Campo - Descida')
plt.xlabel('Passos de tempo')
plt.ylabel('Área queimada')
plt.title('Alfa = 0.078')
plt.ylim(0,0.5)
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig("altitude0.078.png")
plt.clf()