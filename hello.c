#include <linux/init.h>       // Macros para inicialização e limpeza de módulos
#include <linux/module.h>     // Funções específicas para módulos do kernel
#include <linux/kernel.h>     // Macros e funções essenciais do kernel
#include <linux/fs.h>         // Estruturas e funções relacionadas ao sistema de arquivos
#include <asm/uaccess.h>      // Funções para copiar dados entre espaço do usuário e espaço do kernel
#include <linux/vmalloc.h>    // Alocação de memória virtual contígua
#include <linux/slab.h>       // Funções para alocar e liberar blocos de memória
#include <linux/device.h>     // Abstração para representar dispositivos no sistema

// Nome do dispositivo e classe
#define NOME_DISPOSITIVO "echochar"
#define NOME_CLASSE "echo"

MODULE_LICENSE("GPL"); // Licença para o modulo
MODULE_AUTHOR("Gubio Garcia");
MODULE_DESCRIPTION("Driver simples de dispositivo de caractere Linux para echo");
MODULE_VERSION("0.1");

// Parâmetro do módulo para configurar o nome
static char *nome = "echoChar";
module_param(nome, charp, S_IRUGO);
MODULE_PARM_DESC(nome, "O nome a ser exibido no /var/log/kern.log");

// Funções de operação do dispositivo
static int dev_aberto(struct inode *, struct file *);
static int dev_liberar(struct inode *, struct file *);
static ssize_t dev_ler(struct file *, char *, size_t, loff_t *);
static ssize_t dev_escrever(struct file *, const char *, size_t, loff_t *);

// Estrutura que define as operações do dispositivo
static struct file_operations fops = 
{
    .owner = THIS_MODULE,
    .open = dev_aberto,
    .read = dev_ler,
    .write = dev_escrever,
    .release = dev_liberar,
};

// Buffer para armazenar mensagens do usuário
char *mensagem = NULL;
#define TAMANHO_MSG 256

// Função chamada quando o dispositivo é aberto
static int dev_aberto(struct inode *inodep, struct file *filep)
{
    printk(KERN_INFO "EchoChar: Dispositivo foi aberto %d vez(es)\n", 2);

    // Aloca memória para o buffer de mensagem
    mensagem = kmalloc(sizeof(char) * TAMANHO_MSG, GFP_KERNEL);
    return 0;
}

// Função chamada quando o dispositivo é fechado
static int dev_liberar(struct inode *inodep, struct file *filep)
{
    // Libera a memória alocada para o buffer de mensagem
    kfree(mensagem);
    return 0;
}

// Função chamada quando o dispositivo é lido
static ssize_t dev_ler(struct file *filep, char *buf, size_t tamanho, loff_t *offset)
{
    return 0;    
}

// Função chamada quando o dispositivo é escrito
static ssize_t dev_escrever(struct file *filep, const char *buffer, size_t len, loff_t *offset)
{
    // Limpa o buffer de mensagem e copia a mensagem do usuário para o buffer
    memset(mensagem, 0x0, TAMANHO_MSG); 
    sprintf(mensagem, "%s(%zu letras)", buffer, len);
    int nlen = strlen(mensagem);

    printk(KERN_INFO "EchoChar: Recebido %d caracteres do usuario\n", nlen);
    return nlen;
}

// Variáveis globais para armazenar o número principal, a classe e o dispositivo
static int majorNumber;
static struct class *echocharClass = NULL;
static struct device *echocharDevice = NULL;

// Função chamada durante a inicialização do módulo
static int __init hello_init(void)
{
    printk(KERN_INFO "Echo: Ola %s!\n", nome);

    // Registra um número principal para o dispositivo
    majorNumber = register_chrdev(0, NOME_DISPOSITIVO, &fops);
    if (majorNumber < 0)
    {
        printk(KERN_ALERT "EchoChar falhou ao registrar um numero principal\n");
        return majorNumber;
    }

    printk(KERN_INFO "EchoChar: registrado corretamente com numero principal: %d\n", majorNumber);

    // Cria a classe do dispositivo
    echocharClass = class_create(THIS_MODULE, NOME_CLASSE);
    if (IS_ERR(echocharClass))
    {
        unregister_chrdev(majorNumber, NOME_DISPOSITIVO);
        printk(KERN_ALERT "Falha ao registrar a classe do dispositivo\n");
        return PTR_ERR(echocharClass);
    }

    printk(KERN_INFO "EchoChar: classe do dispositivo registrada corretamente\n");

    // Cria o dispositivo
    echocharDevice = device_create(echocharClass, NULL, MKDEV(majorNumber, 0), NULL, NOME_DISPOSITIVO);
    if (IS_ERR(echocharDevice))
    {
        class_destroy(echocharClass);
        unregister_chrdev(majorNumber, NOME_DISPOSITIVO);
        printk(KERN_ALERT "Falha ao criar o dispositivo\n");
        return PTR_ERR(echocharDevice);
    }

    printk(KERN_INFO "EchoChar: dispositivo criado corretamente\n");
    return 0;
}

// Função chamada durante a saída do módulo
static void __exit hello_exit(void)
{
    // Destroi o dispositivo, a classe e registra o número principal
    device_destroy(echocharClass, MKDEV(majorNumber, 0));
    class_unregister(echocharClass);
    class_destroy(echocharClass);
    unregister_chrdev(majorNumber, NOME_DISPOSITIVO);

    printk(KERN_INFO "Echo: Adeus %s!\n", nome);
}

// Indica as funções de inicialização e saída do módulo
module_init(hello_init);
module_exit(hello_exit);

