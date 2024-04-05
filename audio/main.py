from gtts import gTTS
import re

# Função para limpar a frase e torná-la um nome de arquivo válido
def clean_filename(text, max_length=50):
    # Remover caracteres que não são letras, números ou espaços
    text = re.sub(r'[^\w\s]', '', text)
    # Substituir espaços por sublinhados
    text = text.replace(' ', '_')
    # Limitar o comprimento do nome do arquivo
    return text[:max_length]

# Lista de frases em inglês para serem convertidas em áudio
sentences = [
  "The cat is under the table",
  "My patience is running out",
  "Guys say a lot of things",
  "I wait at the bus stop",
  "Here I am another day",
  "I wear my pants",
  "The dog is in the house",
  "The book is on the table",
  "She has a beautiful flower",
  "We are going to the beach",
  "He drives a fast car",
  "I like to listen to music",
  "The sun is shining",
  "The child is playing",
  "She is reading a book",
  "The phone is ringing",
  "I am learning Portuguese",
  "We watch television at night",
  "The fish swims in the water",
  "Birds fly in the sky",
  "He is wearing a watch",
  "The horse is running",
  "She is looking at the mountain",
  "The moon shines at night",
  "He loves playing soccer",
  "She is drinking water",
  "The teacher is speaking",
  "The students are studying",
  "The bird is singing",
  "They are watching a movie",
  "She is cooking dinner"
]

# Gerando e salvando os arquivos de áudio
for sentence in sentences:
    tts = gTTS(text=sentence, lang='en')
    file_name = clean_filename(sentence) + '.mp3'
    tts.save(file_name)
    print(f"Arquivo '{file_name}' gerado com sucesso.")
