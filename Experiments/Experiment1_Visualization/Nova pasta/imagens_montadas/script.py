from PIL import Image
import os

# Configurações
num_linhas = 3
num_colunas = 2
gap_horizontal = 30  # Espaço entre colunas
gap_vertical = 30    # Espaço entre linhas

# Nomes das imagens dos experimentos já montados
imagens_experimentos = [
    "experimento_1.png",
    "experimento_2.png",
    "experimento_3.png",
    "experimento_4.png",
    "experimento_5.png",
    "experimento_6.png"
]

# Verificação básica
if len(imagens_experimentos) != num_linhas * num_colunas:
    raise ValueError("Número de imagens não corresponde ao layout 3x2.")

# Carrega todas as imagens
imagens = [Image.open(img) for img in imagens_experimentos]

# Supondo que todas tenham o mesmo tamanho
largura_individual, altura_individual = imagens[0].size

# Calcula tamanho da imagem final
largura_total = num_colunas * largura_individual + (num_colunas - 1) * gap_horizontal
altura_total = num_linhas * altura_individual + (num_linhas - 1) * gap_vertical

# Cria imagem final em branco
imagem_final = Image.new("RGB", (largura_total, altura_total), color=(255, 255, 255))

# Coloca cada imagem na posição correta
for idx, img in enumerate(imagens):
    linha = idx % num_linhas
    coluna = idx // num_linhas

    x = coluna * (largura_individual + gap_horizontal)
    y = linha * (altura_individual + gap_vertical)

    imagem_final.paste(img, (x, y))

# Salva imagem final
output_path = "experimentos_grid_3x2.png"
imagem_final.save(output_path)
print(f"[OK] Grade final salva como: {output_path}")
