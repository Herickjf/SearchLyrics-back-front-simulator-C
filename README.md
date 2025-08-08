# Projeto Busca de Letras de Música

Este projeto implementa um sistema simples para buscar letras de músicas usando memória compartilhada e pipes em C, com dois processos filhos: um para interação com o usuário e outro para realizar a busca via requisição HTTP (curl).

---

## Criadores

- Herick José  
- Luis Reis  

Ambos estudantes de Ciência da Computação - UFPB.

---

## Descrição

O sistema consiste em:

- Processo cliente: Recebe entrada do usuário (artista e nome da música), escreve essa informação na memória compartilhada e espera a resposta.
- Processo busca: Escuta na memória compartilhada, monta a URL para consulta de letras, faz uma requisição HTTP à API do `lyricmatch.com.br` usando `curl`, e armazena a letra na memória compartilhada para o processo cliente ler.

A comunicação entre os processos é feita por dois pipes para sincronização, e por memória compartilhada para troca dos dados de busca e retorno.

---

## Como funciona

1. O usuário informa o nome do artista e da música.
2. O processo cliente escreve esses dados na memória compartilhada.
3. O processo busca lê esses dados, monta a URL para a API e faz a requisição.
4. A letra obtida é escrita na memória compartilhada.
5. O processo cliente lê e exibe a letra para o usuário.
6. O processo repete enquanto o usuário desejar buscar novas músicas.

---

## Tecnologias e conceitos usados

- Linguagem C
- Processos e fork
- Memória compartilhada (`shm_open`, `mmap`, `shm_unlink`)
- Pipes para comunicação e sincronização
- Requisições HTTP com `curl` via `popen`
- Manipulação básica de strings
- Sincronização entre processos

---

## Como compilar e executar

Compile:

```bash
gcc -o busca_letras main.c -lrt
```

Execute:

```bash
./busca_letras
```

> Nota: É necessário que o comando `curl` esteja disponível no sistema para fazer as requisições HTTP.

---

## Estrutura dos arquivos

* `main.c`: Código fonte principal contendo toda a lógica.

---

## Licença

Este projeto é aberto para estudo e uso acadêmico.

---

## Contato

* [Herick José](https://github.com/Herickjf)
* [Luis Reis](https://github.com/LuisReis09)
  (UFPB - Ciência da Computação)


