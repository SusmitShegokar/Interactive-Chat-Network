import time
import torch
from transformers import AutoTokenizer, AutoModelForCausalLM
import sys

def generate_text(prompt, max_length=200, temperature=0.7, top_p=0.9):
    input_ids = tokenizer.encode(prompt, return_tensors="pt").to(device)
    output = model.generate(input_ids, max_length=max_length, pad_token_id=tokenizer.eos_token_id, no_repeat_ngram_size=3, num_return_sequences=1, top_p=top_p, temperature=temperature, do_sample=True)
    generated_text = tokenizer.decode(output[0], skip_special_tokens=True)
    return generated_text

start = time.time()

# Load pre-trained GPT-2 model and tokenizer
model_name = "openai-community/gpt2-medium"
tokenizer = AutoTokenizer.from_pretrained(model_name)
model = AutoModelForCausalLM.from_pretrained(model_name)

# Set device to GPU if available
device = "cuda" if torch.cuda.is_available() else "cpu"
model.to(device)
print(f'Device: {device}')

# Example prompt
uuid = sys.argv[1]
filename ='chatbot_details_'+uuid+".txt"

prompt = ""
with open(filename, 'r') as file:
    # Read the entire contents of the file
    prompt = file.read()

# Add context to the prompt
context_prompt = ("You are a Chatbot capable of answering questions for airline services. "
                  "Please respond to the following user question to the best of your knowledge "
                  "without generating any follow-up questions and dont just try to complete my sentence. : "
                  )

prompt = context_prompt + prompt

# Generate text based on the prompt
generated_text = generate_text(prompt)

# Print generated text and time taken
end = time.time()

# print("Generated Text:")
# print(generated_text)

# Write generated text to file
generated_text = generated_text.replace(prompt, "")
with open(filename, 'w') as file:
    generated_text = "gpt2bot> " + generated_text
    file.write(generated_text)

print(f'Time taken: {end - start} seconds')