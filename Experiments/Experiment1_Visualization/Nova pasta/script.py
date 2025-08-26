from PIL import Image
import os

# Lista dos sufixos das imagens
nomes_imgs = ['20ts.png', '50ts.png', '100ts.png', '200ts.png', '300ts.png']

# Número de experimentos
num_experimentos = 6  # Ajuste conforme necessário

# Gap (espaço entre as imagens), em pixels
gap = 10

# Pasta de saída
output_dir = "imagens_montadas"
os.makedirs(output_dir, exist_ok=True)

for exp_num in range(1, num_experimentos + 1):
    imagens = []

    for nome_img in nomes_imgs:
        nome_arquivo = f"exp{exp_num}_{nome_img}"
        if os.path.exists(nome_arquivo):
            img = Image.open(nome_arquivo)
            imagens.append(img)
        else:
            print(f"[AVISO] Imagem não encontrada: {nome_arquivo}")

    if len(imagens) != len(nomes_imgs):
        print(f"[ERRO] Experimento {exp_num} está incompleto. Pulando.")
        continue

    # Supondo que todas têm o mesmo tamanho
    largura, altura = imagens[0].size
    num_imgs = len(imagens)
    largura_total = largura * num_imgs + gap * (num_imgs - 1)

    # Cria nova imagem com fundo branco
    imagem_final = Image.new("RGB", (largura_total, altura), color=(255, 255, 255))

    for idx, img in enumerate(imagens):
        x_offset = idx * (largura + gap)
        imagem_final.paste(img, (x_offset, 0))

    output_path = os.path.join(output_dir, f"experimento_{exp_num}.png")
    imagem_final.save(output_path)
    print(f"[OK] Imagem com gap salva: {output_path}")
