#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include "json.h"

#define USEDEBUGPARAM 1
#define IMPORTARCHFROMJSON 1
#define IMPORTPARAMFROMJSON 0
#define LOADDATASET 0

#define WIDTH 224
#define COLORS 3

// LOADING DATA
int loadTrain(int ct, double validRatio, int sh, float imgScale, float imgBias);
int loadTest(int ct, int sh, int rc, float imgScale, float imgBias);
// INIT-NET
void initNet(int t);
void initArch(char *str, int x);
// NEURAL-NET
int isDigits(int init);
void randomizeTrainSet();
void dataAugment(int img, int r, float sc, float dx, float dy, int p, int hiRes, int loRes, int t);
void *runBackProp(void *arg);
int backProp(int x, float *ent, int ep);
int forwardProp(int x, int dp, int train, int lay);
float ReLU(float x);
float TanH(float x);

// TRAINING AND VALIDATION DATA
float (*trainImages)[WIDTH * WIDTH * COLORS] = 0;
float (*trainImages2)[WIDTH * WIDTH * COLORS / 2 / 2] = 0;
int *trainDigits = 0;
int trainSizeI = 0, extraTrainSizeI = 1000;
int trainColumns = 0, trainSizeE = 0;
int *trainSet = 0;
int trainSetSize = 0;
int *validSet = 0;
int validSetSize = 0;
float *ents = 0, *ents2 = 0;
float *accs = 0, *accs2 = 0;
// TEST DATA
float (*testImages)[WIDTH * WIDTH * COLORS] = 0;
float (*testImages2)[WIDTH * WIDTH * COLORS / 2 / 2] = 0;
int *testDigits;
int testSizeI = 0;
int testColumns = 0;

// NETWORK VARIABLES
int inited = -1;
int activation = 1; // 0=Identity, 1=ReLU, 2=TanH
const int randomizeDescent = 1;
float learn = .01;
int DOconv = 1, DOdense = 1, DOpool = 1;
float dropOutRatio = 0.0, decay = 0.95;
float augmentRatio = 1.0, weightScale = 1.0;
float augmentScale = .13, imgBias = 0.0;
int augmentAngle = 13;
float augmentDx = 2.8, augmentDy = 2.8;
// NETWORK ACTIVATIONS AND ERRORS
float prob = 0.0, prob0 = 0.0;
float prob1 = 0.0, prob2 = 0.0;
float *layers[10] = {0};
int *dropOut[10] = {0};
float *weights[10] = {0};
float *errors[10] = {0};
// NETWORK ARCHITECTURE
int numLayers = 0;
char layerNames[10][20] = {0};
int layerType[10] = {0}; // 0FC, 1C, 2P
int layerSizes[10] = {0};
int layerConv[10] = {0};
int layerPad[10] = {0};
int layerWidth[10] = {0};
int layerChan[10] = {0};
int layerStride[10] = {0};
int layerConvStep[10] = {0};
int layerConvStep2[10] = {0};
// THREAD VARIABLES
pthread_t workerThread;
pthread_attr_t stackSizeAttribute;
int pass[5] = {0};
int working = 0;
int requiredStackSize = 8 * 1024 * 1024;
// CONFUSION MATRIX DATA
int maxCD = 54;
int cDigits[10][10][54];
int showAcc = 1;
int showEnt = 1;
int showCon = 0;
int showDig[3][55] = {{0}};
float scaleMin = 0.9, scaleMax = 1.0;
// DOT DATA
const int maxDots = 250;
float trainDots[250][2];
int trainColors[250];
int trainSizeD = 0;
// DOT 3D DISPLAY
int use3D = -1;
float heights3D[121][81] = {{0}};
float pa3D[121][81] = {{0}};
float pb3D[121][81] = {{0}};
float pc3D[121][81] = {{0}};
double *red4 = 0, *green4 = 0, *blue4 = 0;
int requestInit = 0;
// NET ARCHITECTURE
char nets[9][10][20] =
    {{"", "", "", "", "", "", "", "", "", ""},
     {"", "", "", "", "", "", "2", "20", "20", "6"},
     {"", "", "", "150528", "C5:6", "P2", "C5:16", "P2", "128", "2"},
     {"", "150528", "C3:10", "C3:10", "P2", "C3:20", "C3:20", "P2", "128", "2"},
     {"", "", "", "", "", "", "150528", "10", "10", "2"},
     {"", "150528", "C3:32", "P2", "C3:32", "P2", "C3:32", "P2", "512", "2"},
     // debug nets below
     {"", "", "", "", "", "150528", "C5:6", "P2", "50", "2"},
     {"", "", "", "", "", "", "", "37632", "100", "2"},
     {"", "", "", "", "", "16", "C3:2", "P2", "2", "2"}};

pthread_barrier_t barrier;
pthread_barrierattr_t attr;
unsigned count;
int ret;

int intImg[WIDTH * WIDTH * COLORS] = {0};
int *getImg(int index)
{
    double divideBy = 255, subtractBy = 0;
    for (int idx = 0; idx < trainColumns; idx++)
    {
        intImg[idx] = (int)((trainImages[index][idx] + subtractBy) * divideBy);
    }
    return intImg;
}

int getValidInt(int minValue, int maxValue)
{
    int temp_val;
    do
    {
        scanf("%d", &temp_val);
        if (temp_val < minValue || temp_val > maxValue)
        {
            printf("Invalid value, must be between %d and %d. \nPlease retry !\n", minValue, maxValue);
        }
    } while (temp_val < minValue || temp_val > maxValue);
    return temp_val;
}

float getValidFloat(float minValue, float maxValue)
{
    float temp_val;
    do
    {
        scanf("%f", &temp_val);
        if (temp_val < minValue || temp_val > maxValue)
        {
            printf("Invalid value, must be between %f and %f. \nPlease retry !\n", minValue, maxValue);
        }
    } while (temp_val < minValue || temp_val > maxValue);
    return temp_val;
}

/**********************************************************************/
/*      MAIN ROUTINE                                                  */
/**********************************************************************/

int main(int argc, char *argv[])
{
    if (argc > 1)
        printf("Ignoring unknown argument(s)\n");
    srand(time(0));
    pthread_attr_init(&stackSizeAttribute);
    pthread_attr_setstacksize(&stackSizeAttribute, requiredStackSize);

    /**********************************************************************/
    /*      LOADING TRAINING&TEST DATASET                                 */
    /**********************************************************************/
    int rows = 0, removeHeader = 1, removeCol1 = 0;
    double validRatio = 0.2, divideBy = 255, subtractBy = 0;
    if (LOADDATASET)
    {
        int nbRows = loadTrain(rows, validRatio, removeHeader, divideBy, subtractBy);
        printf("Loaded %d rows training, %d features, vSetSize=%d\n", nbRows, trainColumns, validSetSize);
        int test = loadTest(rows, removeHeader, removeCol1, divideBy, subtractBy);
        printf("Loaded %d rows test, %d features\n", test, testColumns);
    }

    /**********************************************************************/
    /*      INIT NET                                                      */
    /**********************************************************************/
    weightScale = 1.414;
    int net = 5;
    printf("Initialized NN=%d with Xavier init scaled=%.3f\n", net, weightScale);
    initNet(net);
    int len = printf("Architecture (%s", layerNames[0]);
    for (int i = 1; i < 10; i++)
        len += printf("-%s", layerNames[i]);
    printf(")\n");

    /**********************************************************************/
    /*      ACTIVATION                                                    */
    /**********************************************************************/
    if (!USEDEBUGPARAM)
    {
        printf("Please choose your activation type where 0=Identity, 1=ReLU, 2=TanH :\n");
        activation = getValidInt(0, 2);
    }
    printf("Activation=%d where 0=Identity, 1=ReLU, 2=TanH\n", activation);

    /**********************************************************************/
    /*      DATA AUGMENTATION                                             */
    /**********************************************************************/
    // AUGMENT RATIO
    if (!USEDEBUGPARAM)
    {
        printf("please choose a ratio for data augment (value between 0,1)\n");
        augmentRatio = getValidFloat(0.0, 1.0);
    }

    // SCALE
    if (!USEDEBUGPARAM)
    {
        printf("please choose a scale for data augment (value between 0,1)\n");
        augmentScale = getValidFloat(0.0, 1.0);
    }

    // ANGLE
    if (!USEDEBUGPARAM)
    {
        printf("please choose a angle for data augment (value between 0-360)\n");
        augmentAngle = getValidInt(0, 360);
    }

    // DX
    if (!USEDEBUGPARAM)
    {
        printf("please choose a DX for data augment (value between 0,10)\n");
        augmentDx = getValidFloat(0.0, 10.0);
    }

    // DY
    if (!USEDEBUGPARAM)
    {
        printf("please choose a DY for data augment (value between 0,10)\n");
        augmentDy = getValidFloat(0.0, 10.0);
    }

    printf("AugmentRatio = %f, Angle = %d, Scale = %.2f, Dx = %.1f, Dy = %.1f\n",
           augmentRatio, augmentAngle, augmentScale, augmentDx, augmentDy);

    /**********************************************************************/
    /*      TRAIN NET                                                     */
    /**********************************************************************/
    int res;
    // learn
    if (!USEDEBUGPARAM)
    {
        printf("please choose an initial learning rate\n");
        learn = getValidFloat(0.0000000001, 1000.0);
    }
    printf("Learning rate set to %f\n", learn);

    // min scale
    if (!USEDEBUGPARAM)
    {
        printf("please enter a min scale, ex 0.95\n");
        scaleMin = getValidFloat(0.01, 1);
    }
    printf("Min scale set to %f\n", scaleMin);

    // max scale
    if (!USEDEBUGPARAM)
    {
        printf("please enter a max scale, ex 1.0\n");
        scaleMax = getValidFloat(0.01, 1);
    }
    printf("Max scale set to %f\n", scaleMax);

    // decay
    if (!USEDEBUGPARAM)
    {
        printf("please enter a decay, ex 0.95\n");
        decay = getValidFloat(0.01, 1);
    }
    printf("Decay set to %f\n", decay);

    // epoch
    int epoch = 20;
    if (!USEDEBUGPARAM)
    {
        printf("please enter an epoch number\n");
        epoch = getValidInt(1, 1000);
    }
    printf("Number of epoch set to %d\n", epoch);

    // display rate
    int displayRate = 1;
    if (!USEDEBUGPARAM)
    {
        printf("please enter a display rate\n");
        displayRate = getValidInt(0, 1000);
    }
    printf("Display rate set to %d\n", displayRate);

    // training
    pass[0] = epoch;
    pass[1] = displayRate;
    pass[2] = 0;
    working = 1;
    printf("Running \n");
    runBackProp(NULL);
    // pthread_create(&workerThread, &stackSizeAttribute, runBackProp, NULL);
    // ret = pthread_join(workerThread, NULL);

    /**********************************************************************/
    /*      PREDICT                                                       */
    /**********************************************************************/

    int NN = 1;
    int big = 1;
    int k = 5;
    int d = 2;
    int dispRate = 100;
    // pthread_cancel(workerThread);

    printf("END OF RUN\n");
}

/**********************************************************************/
/*      LOAD DATA                                                     */
/**********************************************************************/
int loadTrain(int ct, double validRatio, int sh, float imgScale, float imgBias)
{
    char *data;
    // LOAD TRAINING DATA FROM FILE
    if (ct <= 0)
        ct = 1e6;
    int i, len = 0, lines = 1, lines2 = 1;
    float rnd;
    // READ IN TRAIN.CSV
    char buffer[1000000];
    char name[80] = "shipsnet_train.csv";
    if (access(name, F_OK) == 0)
    {
        data = (char *)malloc((unsigned long)fsize(name) + 1);
        FILE *fp;
        fp = fopen(name, "r");
        while (fgets(buffer, 1000000, fp))
        {
            len += sprintf(data + len, "%s", buffer);
            // lines++;
        }
        fclose(fp);
    }
    else
    {
        printf("ERROR: File %s not found.\n", name);
        return 0;
    }
    // COUNT LINES
    for (i = 0; i < len; i++)
    {
        if (data[i] == '\n')
            lines++;
        if (data[i] == '\r')
            lines2++;
    }
    if (lines2 > lines)
        lines = lines2;
    // ALLOCATE MEMORY
    if (trainImages != NULL)
    {
        free(trainImages);
        free(trainImages2);
        free(trainDigits);
        free(trainSet);
        free(validSet);
        trainImages = NULL;
    }
    trainImages = malloc(WIDTH * WIDTH * COLORS * (lines + extraTrainSizeI) * sizeof(float));
    trainImages2 = malloc(WIDTH * WIDTH * COLORS / 2 / 2 * (lines + extraTrainSizeI) * sizeof(float));
    trainDigits = malloc(lines * sizeof(int));
    trainSet = malloc(lines * sizeof(int));
    validSet = malloc(lines * sizeof(int));
    // DECODE COMMA SEPARATED ROWS
    int j = 0, k = 0, c = 0, mark = -1;
    int d = 0, j1, j2;
    while (data[j] != '\n' && data[j] != '\r')
    {
        if (data[j] == ',')
            c++;
        j++;
    }
    if (data[j] != '\n' || data[j] != '\r')
        j++;
    trainColumns = c;
    c = 0;
    i = 0;
    if (sh == 1)
        i = j + 1;
    while (i < len && k < ct)
    {
        j = i;
        while (data[j] != ',' && data[j] != '\r' && data[j] != '\n')
            j++;
        if (data[j] == '\n' || data[j] == '\r')
            mark = 1;
        data[j] = 0;
        d = atof(data + i);
        if (mark == -1)
        {
            trainDigits[k] = (int)d;
            mark = 0;
        }
        else if (mark == 0)
        {
            trainImages[k][c] = d / imgScale - imgBias;
            c++;
        }
        if (mark >= 1)
        {
            trainImages[k][c] = d / imgScale - imgBias;
            if (c >= trainColumns - 1)
                k++;
            c = 0;
            if (j + 1 < len && (data[j + 1] == '\n' || data[j + 1] == '\r'))
                mark++;
            i = j + mark;
            mark = -1;
        }
        else
            i = j + 1;
    }
    validSetSize = 0;
    trainSetSize = 0;
    // CREATE A SUBSAMPLED VERSION OF IMAGES
    if (trainColumns == WIDTH * WIDTH * COLORS)
    {
        for (i = 0; i < k; i++)
        {
            for (j1 = 0; j1 < WIDTH / 2; j1++)
            {
                for (j2 = 0; j2 < WIDTH / 2; j2++)
                {
                    for (int color = 0; color < COLORS; color++)
                    {
                        trainImages2[i][WIDTH / 2 * j1 + j2 * 3 + color] = (trainImages[i][WIDTH * j1 * 2 + j2 * 2 * 3 + color] + trainImages[i][WIDTH * j1 * 2 + (j2 * 2 + 1) * 3 + color] + trainImages[i][WIDTH * (j1 * 2 + 1) + j2 * 2 * 3 + color] + trainImages[i][WIDTH * (j1 * 2 + 1) + (j2 * 2 + 1) * 3 + color]) / 4.0;
                    }
                }
            }
        }
    }
    // CREATE TRAIN AND VALIDATION SETS
    for (i = 0; i < k; i++)
    {
        rnd = (float)rand() / (float)RAND_MAX;
        if (rnd <= validRatio)
            validSet[validSetSize++] = i;
        else
            trainSet[trainSetSize++] = i;
    }
    trainSizeI = k;
    trainSizeE = k;
    free(data);
    return k;
}

/**********************************************************************/
/*      LOAD DATA                                                     */
/**********************************************************************/
int loadTest(int ct, int sh, int rc, float imgScale, float imgBias)
{
    char *data;
    // LOAD TEST DATA FROM FILE
    if (ct <= 0)
        ct = 1e6;
    int i, len = 0, lines = 0, lines2 = 0;
    ;
    float rnd;
    // READ IN TEST.CSV
    char buffer[1000000];
    char name[80] = "shipsnet_test.csv";
    if (access(name, F_OK) == 0)
    {
        data = (char *)malloc((int)fsize(name) + 1);
        FILE *fp;
        fp = fopen(name, "r");
        while (fgets(buffer, 1000000, fp))
        {
            len += sprintf(data + len, "%s", buffer);
            // lines++;
        }
        fclose(fp);
    }
    else
    {
        sprintf(buffer, "ERROR: File %s not found.", name);
        return 0;
    }
    // COUNT LINES
    for (i = 0; i < len; i++)
    {
        if (data[i] == '\n')
            lines++;
        if (data[i] == '\r')
            lines2++;
    }
    if (lines2 > lines)
        lines = lines2;

    // ALLOCATE MEMORY
    if (testImages != NULL)
    {
        free(testImages);
        free(testImages2);
        free(testDigits);
        testImages = NULL;
    }
    testImages = malloc(WIDTH * WIDTH * COLORS * lines * sizeof(float));
    testImages2 = malloc(WIDTH * WIDTH * COLORS / 2 / 2 * lines * sizeof(float));
    testDigits = malloc(lines * sizeof(int));
    // DECODE COMMA SEPARATED ROWS
    int j = 0, k = 0, c = 0, mark = 0;
    int d = 0, j1, j2;
    while (data[j] != '\n' && data[j] != '\r')
    {
        if (data[j] == ',')
            c++;
        j++;
    }
    if (data[j] != '\n' || data[j] != '\r')
        j++;
    testColumns = c + 1;
    if (rc == 1)
    {
        testColumns--;
        mark = -1;
    }
    // printf("len=%d lines=%d columns=%d\n",len,lines,testColumns);
    c = 0;
    i = 0;
    if (sh == 1)
        i = j + 1;
    while (i < len && k < ct)
    {
        j = i;
        while (data[j] != ',' && data[j] != '\r' && data[j] != '\n')
            j++;
        if (data[j] == '\n' || data[j] == '\r')
            mark = 1;
        data[j] = 0;
        d = atof(data + i);
        if (mark == -1)
        {
            mark = 0;
        }
        else if (mark == 0)
        {
            testImages[k][c] = d / imgScale - imgBias;
            c++;
        }
        if (mark >= 1)
        {
            testImages[k][c] = d / imgScale - imgBias;
            if (c >= testColumns - 1)
                k++;
            c = 0;
            if (j + 1 < len && (data[j + 1] == '\n' || data[j + 1] == '\r'))
                mark++;
            i = j + mark;
            mark = 0;
            if (rc == 1)
                mark = -1;
        }
        else
            i = j + 1;
    }
    // CREATE A SUBSAMPLED VERSION OF IMAGES
    if (testColumns == WIDTH * WIDTH * COLORS)
    {
        for (i = 0; i < k; i++)
        {
            for (j1 = 0; j1 < WIDTH / 2; j1++)
            {
                for (j2 = 0; j2 < WIDTH / 2; j2++)
                {
                    for (int color = 0; color < COLORS; color++)
                    {
                        testImages2[i][WIDTH / 2 * j1 + j2 * 3 + color] = (testImages[i][WIDTH * j1 * 2 + j2 * 2 * 3 + color] + testImages[i][WIDTH * j1 * 2 + (j2 * 2 + 1) * 3 + color] + testImages[i][WIDTH * (j1 * 2 + 1) + j2 * 2 * 3 + color] + testImages[i][WIDTH * (j1 * 2 + 1) + (j2 * 2 + 1) * 3 + color]) / 4.0;
                    }
                }
            }
        }
    }
    testSizeI = k;
    free(data);
    return k;
}
/**********************************************************************/
/*      INIT NET                                                      */
/**********************************************************************/
void initArch(char *str, int x)
{
    // PARSES USER INPUT TO CREATE DESIRED NETWORK ARCHITECTURE
    // TODO: remove all spaces, check for invalid characters
    int i;
    char *idx = str, *idx2;
    while (idx[0] == ' ' && idx[0] != 0)
        idx++;
    for (i = 0; i < strlen(idx); i++)
        str[i] = idx[i];
    if (str[0] == 0)
    {
        str[0] = '0';
        str[1] = 0;
    }
    if (str[0] >= '0' && str[0] <= '9') // FULLY CONNECTED
    {
        layerSizes[x] = atoi(str);
        layerConv[x] = 0;
        layerChan[x] = 1;
        layerPad[x] = 0;
        layerWidth[x] = (int)sqrt(layerSizes[x] / 3);
        if (layerWidth[x] * layerWidth[x] != layerSizes[x] / 3)
            layerWidth[x] = 1;
        layerStride[x] = 1;
        layerConvStep[x] = 0;
        layerConvStep2[x] = 0;
        layerType[x] = 0;
    }
    else if (str[0] == 'c' || str[0] == 'C') // CONVOLUTIONNAL LAYER
    {
        int more = 1;
        str[0] = 'C';
        idx = str + 1;
        while (*idx != ':' && *idx != '-' && *idx != 0)
            idx++;
        if (*idx == 0)
            more = 0;
        *idx = 0;
        layerConv[x] = atoi(str + 1);
        layerChan[x] = 1;
        layerPad[x] = 0;
        // layerWidth[x] = layerWidth[x-1];
        layerWidth[x] = layerWidth[x - 1] - layerConv[x] + 1;
        if (more == 1)
        {
            *idx = ':';
            idx++;
            idx2 = idx;
            while (*idx != ':' && *idx != '-' && *idx != 0)
                idx++;
            if (*idx == 0)
                more = 0;
            *idx = 0;
            layerChan[x] = atoi(idx2);
            if (more == 1)
            {
                *idx = ':';
                idx++;
                idx2 = idx;
                while (*idx != ':' && *idx != '-' && *idx != 0)
                    idx++;
                if (*idx == 0)
                    more = 0;
                *idx = 0;
                layerPad[x] = atoi(idx2);
                if (layerPad[x] == 1)
                    layerWidth[x] = layerWidth[x - 1];
                // layerWidth[x] = layerWidth[x-1]-layerConv[x]+1;
            }
        }
        layerSizes[x] = layerWidth[x] * layerWidth[x];
        layerConvStep2[x] = layerConv[x] * layerConv[x];
        layerConvStep[x] = layerConvStep2[x] * layerChan[x - 1];
        layerStride[x] = 1;
        layerType[x] = 1;
    }
    else if (str[0] == 'p' || str[0] == 'P') // POOLING LAYER
    {
        int more = 1;
        if (activation != 0)
            str[0] = 'P'; // allow avg pool if identity act
        idx = str + 1;
        while (*idx != ':' && *idx != '-' && *idx != 0)
            idx++;
        if (*idx == 0)
            more = 0;
        *idx = 0;
        layerConv[x] = atoi(str + 1);
        layerStride[x] = layerConv[x];
        if (more == 1)
        {
            *idx = ':';
            idx++;
            idx2 = idx;
            while (*idx != ':' && *idx != '-' && *idx != 0)
                idx++;
            if (*idx == 0)
                more = 0;
            *idx = 0;
            layerStride[x] = atoi(idx2);
        }
        int newWidth = layerWidth[x - 1] / layerStride[x];
        if (layerStride[x] != layerConv[x])
            newWidth = (layerWidth[x - 1] - layerConv[x] + layerStride[x]) / layerStride[x];
        layerSizes[x] = newWidth * newWidth;
        layerChan[x] = layerChan[x - 1];
        layerPad[x] = 0;
        layerWidth[x] = newWidth;
        layerConvStep2[x] = layerConv[x] * layerConv[x];
        layerConvStep[x] = layerConvStep2[x];
        layerType[x] = 2; // MAX POOLING
        if (str[0] == 'p')
            layerType[x] = 3; // AVG POOLING
    }
    strcpy(layerNames[x], str);
}

/**********************************************************************/
/*      INIT NET                                                      */
/**********************************************************************/
void initNet(int t)
{
    // ALLOCATION MEMORY AND INITIALIZE NETWORK WEIGHTS
    int i, idxLayer, same = 1, LL, dd = 9;
    char buf[10], buf2[20];

    // FREE OLD NET
    if ((t != inited && layers[0] != NULL) || (t == 0 && same == 0))
    {
        free(layers[0]);
        free(errors[0]);
        for (i = 1; i < 10; i++)
        {
            free(layers[i]);
            free(dropOut[i]);
            free(errors[i]);
            free(weights[i]);
        }
        layers[0] = NULL;
    }
    // SET NEW NET ARCHITECTURE

    numLayers = 0;

    if (IMPORTARCHFROMJSON)
    {
        char filename[] = "example_file.json";
        char *buff2 = (char *)malloc((unsigned long)fsize(filename) + 1);
        ret = read_from_file(filename, buff2);

        int size = get_int_in_json(buff2, "size");

        char *tab = get_tab_in_json(buff2, "arch");

        char archCNN[size][20];
        for (int i = 0; i < size; i++)
        {
            memcpy(archCNN[i], "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", 20);
            char *str_in_tab = get_str_in_tab(tab, i);
            strcpy(archCNN[i], str_in_tab);
            free(str_in_tab);
        }
        for (i = 0; i < 10; i++)
        {
            initArch(archCNN[i], i);
            sprintf(buf, "L%d", i);
            if (numLayers == 0 && layerSizes[i] != 0)
                numLayers = 10 - i;
        }
    }
    else
    {
        for (i = 0; i < 10; i++)
        {
            initArch(nets[t][i], i);
            sprintf(buf, "L%d", i);
            if (numLayers == 0 && layerSizes[i] != 0)
                numLayers = 10 - i;
        }
    }
    sprintf(buf, "L%d", i);
    if (numLayers == 0 && layerSizes[i] != 0)
        numLayers = 10 - i;
    // printf("\n");

    // ALOCATE MEMORY
    if (layers[0] == NULL)
    {
        layers[0] = (float *)malloc((layerSizes[0] + 1) * sizeof(float));
        errors[0] = (float *)malloc(layerSizes[0] * sizeof(float));
        for (i = 1; i < 10; i++)
        {
            layers[i] = (float *)malloc((layerSizes[i] * layerChan[i] + 1) * sizeof(float));
            dropOut[i] = (int *)malloc((layerSizes[i] * layerChan[i] + 1) * sizeof(int));
            // printf("setting dropOut i=%d to %d\n",i,(layerSizes[i] * layerChan[i] + 1));
            errors[i] = (float *)malloc((layerSizes[i] * layerChan[i] + 1) * sizeof(float));
            if (layerType[i] == 0) // FULLY CONNECTED
                weights[i] = (float *)malloc(layerSizes[i] * (layerSizes[i - 1] * layerChan[i - 1] + 1) * sizeof(float));
            else if (layerType[i] == 1) // CONVOLUTION
                weights[i] = (float *)malloc((layerConvStep[i] + 1) * layerChan[i] * sizeof(float));
            else if (layerType[i] >= 2) // POOLING (2=max, 3=avg)
                weights[i] = (float *)malloc(sizeof(float));
        }
    }
    // RANDOMIZE WEIGHTS AND BIAS
    float scale;
    for (i = 0; i < 10; i++)
        layers[i][layerSizes[i] * layerChan[i]] = 1.0;
    for (idxLayer = 1; idxLayer < 10; idxLayer++)
    {
        scale = 1.0;
        if (layerSizes[idxLayer - 1] != 0)
        {
            // XAVIER INITIALIZATION (= SQRT( 6/(N_in + N_out) ) ) What is N_out to MaxPool ??
            if (layerType[idxLayer] == 0)
            { // FC LAYER
                if (layerType[idxLayer + 1] == 0)
                    scale = 2.0 * sqrt(6.0 / (layerSizes[idxLayer - 1] * layerChan[idxLayer - 1] + layerSizes[idxLayer]));
                else if (layerType[idxLayer + 1] == 1) // impossible
                    scale = 2.0 * sqrt(6.0 / (layerSizes[idxLayer - 1] * layerChan[idxLayer - 1] + layerConvStep[idxLayer + 1]));
                else if (layerType[idxLayer + 1] >= 2) // impossible
                    scale = 2.0 * sqrt(6.0 / (layerSizes[idxLayer - 1] * layerChan[idxLayer - 1] + layerSizes[idxLayer - 1] * layerChan[idxLayer - 1]));
            }
            else if (layerType[idxLayer] == 1)
            { // CONV LAYER
                if (layerType[idxLayer + 1] == 0)
                    scale = 2.0 * sqrt(6.0 / (layerConvStep[idxLayer] + layerSizes[idxLayer] * layerChan[idxLayer]));
                else if (layerType[idxLayer + 1] == 1)
                    scale = 2.0 * sqrt(6.0 / (layerConvStep[idxLayer] + layerConvStep[idxLayer + 1]));
                else if (layerType[idxLayer + 1] >= 2)
                    scale = 2.0 * sqrt(6.0 / (layerConvStep[idxLayer] + layerConvStep[idxLayer]));
            }
            // if (activation==1 && j!=9) scale *= sqrt(2.0); // DO I WANT THIS? INPUT ISN'T MEAN=0
            // printf("Init layer %d: LS=%d LC=%d LCS=%d Scale=%f\n",j,layerSizes[j],layerChan[j],layerConvStep[j],scale);
            if (idxLayer != 9)
                scale *= weightScale;
        }
        if (layerType[idxLayer] == 0)
        { // FULLY CONNECTED
            if (IMPORTPARAMFROMJSON){
                return ; // TODO
            }
            else {
                for (i = 0; i < layerSizes[idxLayer] * (layerSizes[idxLayer - 1] * layerChan[idxLayer - 1] + 1); i++)
                    weights[idxLayer][i] = scale * ((float)rand() / (float)RAND_MAX - 0.5);
                // weights[j][i] = 0.1;
                for (i = 0; i < layerSizes[idxLayer]; i++) // set biases to zero
                    weights[idxLayer][(layerSizes[idxLayer - 1] * layerChan[idxLayer - 1] + 1) * (i + 1) - 1] = 0.0;
            }
        }
        else if (layerType[idxLayer] == 1)
        { // CONVOLUTION
            if(IMPORTPARAMFROMJSON){
                return NULL; // TODO
            }
            else{
                for (i = 0; i < (layerConvStep[idxLayer] + 1) * layerChan[idxLayer]; i++)
                    weights[idxLayer][i] = scale * ((float)rand() / (float)RAND_MAX - 0.5);
                for (i = 0; i < layerChan[idxLayer]; i++) // set conv biases to zero
                    weights[idxLayer][(layerConvStep[idxLayer] + 1) * (i + 1) - 1] = 0.0;
            }
        }
    }

    inited = t;
    if (isDigits(inited) != 1)
    {
        showCon = 0;
        showDig[0][0] = 0;
    }
}

/**********************************************************************/
/*      NEURAL NETWORK                                                */
/**********************************************************************/
int isDigits(int init)
{
    // DETERMINES WHETHER TO TRAIN DOTS OR LOADED DATA
    int in = 10 - numLayers;
    if (layerSizes[in] == 196 || layerSizes[in] == 784 || layerSizes[in] == trainColumns)
        return 1;
    else
        return 0;
}

/**********************************************************************/
/*      NEURAL NETWORK                                                */
/**********************************************************************/
void randomizeTrainSet()
{
    // RANDOMIZES INDICES IN TRAINING SET
    int i, temp, x;
    for (i = 0; i < trainSetSize; i++)
    {
        x = (int)(trainSetSize * ((float)rand() / (float)RAND_MAX) - 1);
        temp = trainSet[i];
        trainSet[i] = trainSet[x];
        trainSet[x] = temp;
    }
}

/**********************************************************************/
/*      NEURAL NETWORK                                                */
/**********************************************************************/
void dataAugment(int img, int r, float sc, float dx, float dy, int p, int hiRes, int loRes, int t)
{
    // AUGMENT AN IMAGE AND STORE RESULT IN TRAIN IMAGES ARRAY
    int i, j, x2, y2;
    float x, y;
    float pi = 3.1415926;
    float c = cos(pi * r / 180.0);
    float s = sin(pi * r / 180.0);
    float(*trainImagesB)[WIDTH * WIDTH * COLORS] = trainImages;
    float(*trainImages2B)[WIDTH * WIDTH * COLORS / 2 / 2] = trainImages2;
    if (t == 0)
    {
        trainImagesB = testImages;
        trainImages2B = testImages2;
    }
    if (loRes == 1)
    {
        for (i = 0; i < WIDTH / 2; i++)
            for (j = 0; j < WIDTH / 2; j++)
            {
                x = (j - 6.5) / sc - dx / 2.0;
                y = (i - 6.5) / sc + dy / 2.0;
                x2 = (int)round((c * x - s * y) + 6.5);
                y2 = (int)round((s * x + c * y) + 6.5);
                if (y2 >= 0 && y2 < WIDTH / 2 && x2 >= 0 && x2 < WIDTH / 2)
                    trainImages2[trainSizeE][i * WIDTH / 2 + j] = trainImages2B[img][y2 * WIDTH / 2 + x2];
                else
                    trainImages2[trainSizeE][i * WIDTH / 2 + j] = -imgBias;
            }
    }
    if (hiRes == 1)
    {
        for (i = 0; i < WIDTH; i++)
            for (j = 0; j < WIDTH; j++)
            {
                x = (j - 13.5) / sc - dx;
                y = (i - 13.5) / sc + dy;
                x2 = (int)round((c * x - s * y) + 13.5);
                y2 = (int)round((s * x + c * y) + 13.5);
                if (y2 >= 0 && y2 < WIDTH && x2 >= 0 && x2 < WIDTH)
                    trainImages[trainSizeE][i * WIDTH + j] = trainImagesB[img][y2 * WIDTH + x2];
                else
                    trainImages[trainSizeE][i * WIDTH + j] = -imgBias;
            }
    }
    trainDigits[trainSizeE] = trainDigits[img];
    if (t == 0)
        trainDigits[trainSizeE] = -1;
}

/**********************************************************************/
/*      NEURAL NETWORK                                                */
/**********************************************************************/
void *runBackProp(void *arg)
{
    // TRAINS NEURAL NETWORK WITH TRAINING DATA
    time_t start, stop;
    showEnt = 1;
    showAcc = 1;
    char buffer[80];
    int i, x = pass[0], y = pass[1], z = pass[2];
    int p, confusion[10][10] = {{0}};
    if (layers[0] == NULL)
    {
        initNet(1);
        if (z == 1)
        {
            printf("Assuming NN=1 with Xavier init scaled=%.3f", weightScale);
        }
        int len = printf("Architecture (%d", layerSizes[0]);
        for (i = 1; i < 10; i++)
            len += printf("-%d", layerSizes[i]);
        printf(")\n");
    }
    // LEARN DIGITS
    int trainSize = trainSetSize;
    int testSize = validSetSize;
    if (isDigits(inited) == 1)
    {
        showCon = 1;
    }
    else
    { // LEARN DOTS
        trainSize = trainSizeD;
        testSize = 0;
    }
    if (trainSize == 0)
    {
        if (isDigits(inited) == 1)
            printf("Load images first. Click load.");
        else
            printf("Create training dots first. Click dots inside pane to the right.");
        working = 0;
        return NULL;
    }
    // ALLOCATE MEMORY FOR ENTORPY AND ACCURACY HISTORY
    if (ents != NULL)
    {
        free(ents);
        free(ents2);
        free(accs);
        free(accs2);
        ents = NULL;
    }
    ents = (float *)malloc((int)(x / y + 1) * sizeof(float));
    ents2 = (float *)malloc((int)(x / y + 1) * sizeof(float));
    accs = (float *)malloc((int)(x / y + 1) * sizeof(float));
    accs2 = (float *)malloc((int)(x / y + 1) * sizeof(float));
    int entSize = 0, accSize = 0, ent2Size = 0, acc2Size = 0;
    int j, j2, k, s, s2, b;
    float entropy, entropy2, ent;
    if (isDigits(inited) == 1 && randomizeDescent == 1)
        randomizeTrainSet();
    time(&start);
    // PERFORM X TRAINING EPOCHS
    for (j = 0; j < x; j++)
    {
        s = 0;
        entropy = 0.0;
        if (isDigits(inited) != 1)
            trainSize = trainSizeD;
        for (i = 0; i < trainSize; i++)
        {
            // if (i%100==0) printf("x=%d, i=%d\n",j,i);
            if (isDigits(inited) == 1)
                b = backProp(trainSet[i], &ent, j); // LEARN DIGITS
            else
                b = backProp(i, &ent, 0); // LEARN DOTS
            if (b == -1)
            {
                if (z == 1)
                    printf("Exploded. Lower learning rate.\n");
                else
                    printf("Exploded. Lower learning rate.\n");
                working = 0;
                return NULL;
            }
            s += b;
            entropy += ent;
            if (working == 0)
            {
                printf("learning stopped early\n");
                pthread_exit(NULL);
            }
        }
        entropy = entropy / trainSize;
        s2 = 0;
        entropy2 = 0.0;
        for (i = 0; i < 10; i++)
            for (k = 0; k < 10; k++)
                confusion[i][k] = 0;
        for (i = 0; i < 10; i++)
            for (j2 = 0; j2 < 10; j2++)
                for (k = 0; k < maxCD; k++)
                    cDigits[i][j2][k] = -1;
        for (i = 0; i < testSize; i++)
        {
            p = forwardProp(validSet[i], 0, 1, 0);
            if (p == -1)
            {
                if (z == 1)
                    printf("Test exploded.\n");
                else
                    printf("Test exploded.\n");
                working = 0;
                return NULL;
            }
            if (p == trainDigits[validSet[i]])
                s2++;
            cDigits[trainDigits[validSet[i]]][p][confusion[trainDigits[validSet[i]]][p] % maxCD] = validSet[i];
            confusion[trainDigits[validSet[i]]][p]++;
            if (layers[9][p] == 0)
            {
                if (z == 1)
                    printf("Test vanished.\n");
                else
                    printf("Test vanished.\n");
                working = 0;
                return NULL;
            }
            entropy2 -= log(layers[9][p]);
            if (working == 0)
            {
                printf("learning stopped early\n");
                pthread_exit(NULL);
            }
        }
        entropy2 = entropy2 / testSize;
        if (j == 0 || (j + 1) % y == 0)
        {
            ents[entSize++] = entropy;
            accs[accSize++] = (float)s / trainSize;
            if (isDigits(inited) == 1)
            {
                accs2[acc2Size++] = (float)s2 / testSize;
                ents2[ent2Size++] = entropy2;
            }
            time(&stop);
            printf("i=%d acc=%d/%d, ent=%.4f, lr=%.1e\n", j + 1, s, trainSize, entropy, learn * pow(decay, j));
            if (isDigits(inited) == 1 && testSize > 0)
                printf("i=%d train=%.2f ent=%.4f,valid=%.2f ent=%.4f (%.0fsec) lr=%.1e\n",
                       j + 1, 100.0 * s / trainSize, entropy, 100.0 * s2 / testSize, entropy2, difftime(stop, start), learn * pow(decay, j));
            else if (isDigits(inited) == 1 && testSize == 0)
                printf("i=%d train=%.2f ent=%.4f (%.0fsec) lr=%.1e\n",
                       j + 1, 100.0 * s / trainSize, entropy, difftime(stop, start), learn * pow(decay, j));
            time(&start);
            // if (z==1 && isDigits(inited)!=1) {
            //     if (use3D==1) displayClassify3D();
            //     else displayClassify(0);
            // }
            // if (z==1 && showEnt==1) displayEntropy(ents,entSize,ents2,y);
            // if (z==1 && showAcc==1) displayAccuracy(accs,accSize,accs2,y);
            // if (z==1 && isDigits(inited)==1 && showCon==1)  displayConfusion(confusion);
        }
        if (requestInit == 1)
        {
            requestInit = 0;
        }
        if (working == 0)
        {
            printf("learning stopped early\n");
            pthread_exit(NULL);
        }
    }
    printf("Done\n");
    working = 0;
    return NULL;
}

/**********************************************************************/
/*      NEURAL NETWORK                                                */
/**********************************************************************/
int backProp(int x, float *ent, int ep)
{
    // BACK PROPAGATION WITH 1 TRAINING IMAGE
    int i = 0, j, k, r = 0, d = 0, rot = 0, hres = 0, lres = 1;
    float der = 1.0, xs = 0.0, ys = 0.0, extra = 0.0, sc = 1.0, sum;
    int dc, a, a2, i2, j2, i3, j3, pmax, imax, jmax;
    int temp, temp2;
    // DATA AUGMENTATION
    if (augmentRatio > 0.0 && isDigits(inited) == 1)
        if ((float)rand() / (float)RAND_MAX <= augmentRatio)
        {
            if (augmentAngle > 0.0)
                rot = (int)(2.0 * augmentAngle * (float)rand() / (float)RAND_MAX - augmentAngle);
            if (augmentDx > 0.0)
                xs = (2.0 * augmentDx * (float)rand() / (float)RAND_MAX - augmentDx);
            if (augmentDy > 0.0)
                ys = (2.0 * augmentDy * (float)rand() / (float)RAND_MAX - augmentDy);
            if (augmentScale > 0.0)
                sc = 1.0 + 2.0 * augmentScale * (float)rand() / (float)RAND_MAX - augmentScale;
            if (layerSizes[10 - numLayers] == WIDTH * WIDTH * COLORS)
            {
                hres = 1;
                lres = 0;
            }
            dataAugment(x, rot, sc, xs, ys, -1, hres, lres, 1);
            x = trainSizeE;
        }
    // FORWARD PROP FIRST
    int p = forwardProp(x, 1, 1, 0);
    if (p == -1)
        return -1; // GRADIENT EXPLODED
    // CORRECT PREDICTION?
    int y;
    if (isDigits(inited) == 1)
        y = trainDigits[x];
    else
        y = trainColors[x];
    if (p == y)
        r = 1;
    // OUTPUT LAYER - CALCULATE ERRORS
    for (i = 0; i < layerSizes[9]; i++)
    {
        errors[9][i] = learn * (0 - layers[9][i]) * pow(decay, ep);
        if (i == y)
        {
            errors[9][i] = learn * (1 - layers[9][i]) * pow(decay, ep);
            if (layers[9][i] == 0)
                return -1; // GRADIENT VANISHED
            *ent = -log(layers[9][i]);
        }
    }
    // HIDDEN LAYERS - CALCULATE ERRORS
    for (k = 8; k > 10 - numLayers; k--)
    {
        if (layerType[k + 1] == 0) // FEEDS INTO FULLY CONNECTED
            for (i = 0; i < layerSizes[k] * layerChan[k]; i++)
            {
                errors[k][i] = 0.0;
                if (dropOutRatio == 0.0 || DOdense == 0 || dropOut[k][i] == 1)
                { // dropout
                    if (activation == 2)
                        der = (layers[k][i] + 1) * (1 - layers[k][i]); // TanH derivative
                    if (activation == 0 || activation == 2 || layers[k][i] > 0)
                    { // this is ReLU derivative
                        temp = layerSizes[k] * layerChan[k] + 1;
                        for (j = 0; j < layerSizes[k + 1]; j++)
                            errors[k][i] += errors[k + 1][j] * weights[k + 1][j * temp + i] * der;
                    }
                }
            }
        else if (layerType[k + 1] == 1)
        { // FEEDS INTO CONVOLUTION
            for (i = 0; i < layerSizes[k] * layerChan[k]; i++)
                errors[k][i] = 0.0;
            dc = 0;
            if (layerPad[k + 1] == 1)
                dc = layerConv[k + 1] / 2;
            for (a = 0; a < layerChan[k + 1]; a++)
                for (i = 0; i < layerWidth[k + 1]; i++)
                    for (j = 0; j < layerWidth[k + 1]; j++)
                    {
                        temp = a * (layerConvStep[k + 1] + 1);
                        temp2 = a * layerSizes[k + 1] + i * layerWidth[k + 1] + j;
                        for (a2 = 0; a2 < layerChan[k]; a2++)
                            for (i2 = 0; i2 < layerConv[k + 1]; i2++)
                                for (j2 = 0; j2 < layerConv[k + 1]; j2++)
                                {
                                    i3 = i + i2 - dc;
                                    j3 = j + j2 - dc;
                                    if (activation == 2)
                                        der = (layers[k][a2 * layerSizes[k] + i3 * layerWidth[k] + j3] + 1) * (1 - layers[k][a2 * layerSizes[k] + i3 * layerWidth[k] + j3]); // TanH
                                    if (activation == 0 || activation == 2 || layers[k][a2 * layerSizes[k] + i3 * layerWidth[k] + j3] > 0)                                   // this is ReLU derivative
                                        if (i3 >= 0 && i3 < layerWidth[k] && j3 >= 0 && j3 < layerWidth[k])                                                                  // padding
                                            errors[k][a2 * layerSizes[k] + i3 * layerWidth[k] + j3] +=
                                                weights[k + 1][temp + a2 * layerConvStep2[k + 1] + i2 * layerConv[k + 1] + j2] * errors[k + 1][temp2] * der;
                                }
                    }
            if (dropOutRatio > 0.0 && DOconv == 1) // dropout
                for (i = 0; i < layerSizes[k] * layerChan[k]; i++)
                    errors[k][i] = errors[k][i] * dropOut[k][i];
        }
        else if (layerType[k + 1] >= 2)
        { // FEEDS INTO POOLING (2=max, 3=avg)
            for (i = 0; i < layerSizes[k] * layerChan[k]; i++)
                errors[k][i] = 0.0;
            for (a = 0; a < layerChan[k]; a++)
                for (i = 0; i < layerWidth[k + 1]; i++)
                    for (j = 0; j < layerWidth[k + 1]; j++)
                    {
                        pmax = -1e6;
                        if (layerType[k + 1] == 3)
                            temp = errors[k + 1][a * layerSizes[k + 1] + i * layerWidth[k + 1] + j] / layerConvStep2[k + 1];
                        for (i2 = 0; i2 < layerConv[k + 1]; i2++)
                            for (j2 = 0; j2 < layerConv[k + 1]; j2++)
                            {
                                if (layerType[k + 1] == 3)
                                    errors[k][a * layerSizes[k] + (i * layerStride[k + 1] + i2) * layerWidth[k] + j * layerStride[k + 1] + j2] = temp;
                                else if (layers[k][a * layerSizes[k] + (i * layerStride[k + 1] + i2) * layerWidth[k] + j * layerStride[k + 1] + j2] > pmax)
                                {
                                    pmax = layers[k][a * layerSizes[k] + (i * layerStride[k + 1] + i2) * layerWidth[k] + j * layerStride[k + 1] + j2];
                                    imax = i2;
                                    jmax = j2;
                                }
                            }
                        if (layerType[k + 1] == 2)
                            errors[k][a * layerSizes[k] + (i * layerStride[k + 1] + imax) * layerWidth[k] + j * layerStride[k + 1] + jmax] =
                                errors[k + 1][a * layerSizes[k + 1] + i * layerWidth[k + 1] + j];
                    }
            if (dropOutRatio > 0.0 && DOpool == 1) // dropout
                for (i = 0; i < layerSizes[k] * layerChan[k]; i++)
                    errors[k][i] = errors[k][i] * dropOut[k][i];
        }
    }

    // UPDATE WEIGHTS - GRADIENT DESCENT
    int count = 0;
    for (k = 11 - numLayers; k < 10; k++)
    {
        if (layerType[k] == 0)
        { // FULLY CONNECTED LAYER
            for (i = 0; i < layerSizes[k]; i++)
            {
                temp = i * (layerSizes[k - 1] * layerChan[k - 1] + 1);
                for (j = 0; j < layerSizes[k - 1] * layerChan[k - 1] + 1; j++)
                    weights[k][temp + j] += errors[k][i] * layers[k - 1][j];
            }
        }
        else if (layerType[k] == 1)
        { // CONVOLUTION LAYER
            dc = 0;
            if (layerPad[k] == 1)
                dc = layerConv[k] / 2;
            for (a = 0; a < layerChan[k]; a++)
                for (i = 0; i < layerWidth[k]; i++)
                    for (j = 0; j < layerWidth[k]; j++)
                    {
                        temp = a * (layerConvStep[k] + 1);
                        temp2 = a * layerSizes[k] + i * layerWidth[k] + j;
                        for (a2 = 0; a2 < layerChan[k - 1]; a2++)
                            for (i2 = 0; i2 < layerConv[k]; i2++)
                                for (j2 = 0; j2 < layerConv[k]; j2++)
                                {
                                    i3 = i + i2 - dc;
                                    j3 = j + j2 - dc;
                                    if (i3 >= 0 && i3 < layerWidth[k - 1] && j3 >= 0 && j3 < layerWidth[k - 1])
                                        weights[k][temp + a2 * layerConvStep2[k] + i2 * layerConv[k] + j2] +=
                                            errors[k][temp2] * layers[k - 1][a2 * layerSizes[k - 1] + i3 * layerWidth[k - 1] + j3];
                                }
                        weights[k][(a + 1) * (layerConvStep[k] + 1) - 1] += errors[k][a * layerSizes[k] + i * layerWidth[k] + j];
                    }
        }
    }
    return r;
}

/**********************************************************************/
/*      NEURAL NETWORK                                                */
/**********************************************************************/
int forwardProp(int image, int dp, int train, int lay)
{
    // FORWARD PROPAGATION WITH 1 IMAGE
    int i, j, layer, imax, dc;
    int a, a2, i2, j2, i3, j3;
    float sum, esum, max, rnd, pmax;
    int temp, temp2;
    // INPUT LAYER
    if (isDigits(inited) == 1 && layerSizes[10 - numLayers] == WIDTH * WIDTH * COLORS / 2 / 2)
    {
        if (train == 1)
            for (i = 0; i < WIDTH * WIDTH * COLORS / 2 / 2; i++)
                layers[10 - numLayers][i] = trainImages2[image][i];
        else
            for (i = 0; i < WIDTH * WIDTH * COLORS / 2 / 2; i++)
                layers[10 - numLayers][i] = testImages2[image][i];
    }
    else if (isDigits(inited) == 1 && layerSizes[10 - numLayers] == WIDTH * WIDTH * COLORS)
    {
        if (train == 1)
            for (i = 0; i < WIDTH * WIDTH * COLORS; i++)
                layers[10 - numLayers][i] = trainImages[image][i];
        else
            for (i = 0; i < WIDTH * WIDTH * COLORS; i++)
                layers[10 - numLayers][i] = testImages[image][i];
    }
    else if (isDigits(inited) == 1 && layerSizes[10 - numLayers] == trainColumns)
    {
        if (train == 1)
            for (i = 0; i < trainColumns; i++)
                layers[10 - numLayers][i] = trainImages[image][i];
        else
            for (i = 0; i < trainColumns; i++)
                layers[10 - numLayers][i] = testImages[image][i];
    }

    // HIDDEN LAYERS
    for (layer = 11 - numLayers; layer < 9; layer++)
    {
        if (lay != 0 && layer > lay)
            return -1;
        // CALCULATE DROPOUT
        // if (dropOutRatio>0.0) // ALWAYS SET TO 1 TO BE SAFE
        for (i = 0; i < layerSizes[layer] * layerChan[layer]; i++)
        {
            dropOut[layer][i] = 1;
            if (dropOutRatio > 0.0 && dp == 1)
            {
                rnd = (float)rand() / (float)RAND_MAX;
                if (rnd < dropOutRatio)
                    dropOut[layer][i] = 0;
            }
        }

        if (layerType[layer] == 0) // FULLY CONNECTED LAYER
            for (i = 0; i < layerSizes[layer]; i++)
            {
                if (dropOutRatio == 0.0 || dp == 0 || DOdense == 0 || dropOut[layer][i] == 1)
                {
                    temp = i * (layerSizes[layer - 1] * layerChan[layer - 1] + 1);
                    sum = 0.0;
                    for (j = 0; j < layerSizes[layer - 1] * layerChan[layer - 1] + 1; j++)
                        sum += layers[layer - 1][j] * weights[layer][temp + j];
                    if (activation == 0)
                        layers[layer][i] = sum;
                    else if (activation == 1)
                        layers[layer][i] = ReLU(sum);
                    else
                        layers[layer][i] = TanH(sum);
                    // if (dropOutRatio>0.0 && dp==1) layers[layer][i] = layers[layer][i]  / (1-dropOutRatio);
                    if (dropOutRatio > 0.0 && dp == 0 && DOdense == 1)
                        layers[layer][i] = layers[layer][i] * (1 - dropOutRatio);
                }
                else
                    layers[layer][i] = 0.0;
            }
        else if (layerType[layer] == 1)
        { // CONVOLUTION LAYER
            dc = 0;
            if (layerPad[layer] == 1)
                dc = layerConv[layer] / 2;
            for (a = 0; a < layerChan[layer]; a++)
                for (i = 0; i < layerWidth[layer]; i++)
                    for (j = 0; j < layerWidth[layer]; j++)
                    {
                        temp = a * (layerConvStep[layer] + 1);
                        sum = 0.0;
                        for (a2 = 0; a2 < layerChan[layer - 1]; a2++)
                            for (i2 = 0; i2 < layerConv[layer]; i2++)
                                for (j2 = 0; j2 < layerConv[layer]; j2++)
                                {
                                    i3 = i + i2 - dc;
                                    j3 = j + j2 - dc;
                                    if (i3 >= 0 && i3 < layerWidth[layer - 1] && j3 >= 0 && j3 < layerWidth[layer - 1])
                                        sum += layers[layer - 1][a2 * layerSizes[layer - 1] + i3 * layerWidth[layer - 1] + j3] * weights[layer][temp + a2 * layerConvStep2[layer] + i2 * layerConv[layer] + j2];
                                    else
                                        sum -= imgBias * weights[layer][temp + a2 * layerConvStep2[layer] + i2 * layerConv[layer] + j2];
                                }
                        sum += weights[layer][(a + 1) * (layerConvStep[layer] + 1) - 1];
                        if (activation == 0)
                            layers[layer][a * layerSizes[layer] + i * layerWidth[layer] + j] = sum;
                        else if (activation == 1)
                            layers[layer][a * layerSizes[layer] + i * layerWidth[layer] + j] = ReLU(sum);
                        else
                            layers[layer][a * layerSizes[layer] + i * layerWidth[layer] + j] = TanH(sum);
                    }
            // APPLY DROPOUT
            if (dropOutRatio > 0.0 && DOconv == 1)
                for (i = 0; i < layerSizes[layer] * layerChan[layer]; i++)
                {
                    if (dp == 0)
                        layers[layer][i] = layers[layer][i] * (1 - dropOutRatio);
                    else if (dp == 1)
                        layers[layer][i] = layers[layer][i] * dropOut[layer][i];
                }
        }
        else if (layerType[layer] >= 2) // POOLING LAYER (2=max, 3=avg)
            for (a = 0; a < layerChan[layer]; a++)
                for (i = 0; i < layerWidth[layer]; i++)
                    for (j = 0; j < layerWidth[layer]; j++)
                    {
                        sum = 0.0;
                        pmax = -1e6;
                        for (i2 = 0; i2 < layerConv[layer]; i2++)
                            for (j2 = 0; j2 < layerConv[layer]; j2++)
                            {
                                if (layerType[layer] == 3)
                                    sum += layers[layer - 1][a * layerSizes[layer - 1] + (i * layerStride[layer] + i2) * layerWidth[layer - 1] + j * layerStride[layer] + j2];
                                else if (layers[layer - 1][a * layerSizes[layer - 1] + (i * layerStride[layer] + i2) * layerWidth[layer - 1] + j * layerStride[layer] + j2] > pmax)
                                    pmax = layers[layer - 1][a * layerSizes[layer - 1] + (i * layerStride[layer] + i2) * layerWidth[layer - 1] + j * layerStride[layer] + j2];
                            }
                        if (layerType[layer] == 3)
                            layers[layer][a * layerSizes[layer] + i * layerWidth[layer] + j] = sum / layerConvStep2[layer];
                        else
                            layers[layer][a * layerSizes[layer] + i * layerWidth[layer] + j] = pmax;
                    }
        // APPLY DROPOUT
        if (dropOutRatio > 0.0 && DOpool == 1)
            for (i = 0; i < layerSizes[layer] * layerChan[layer]; i++)
            {
                if (dp == 0)
                    layers[layer][i] = layers[layer][i] * (1 - dropOutRatio);
                else if (dp == 1)
                    layers[layer][i] = layers[layer][i] * dropOut[layer][i];
            }
    }

    // OUTPUT LAYER - SOFTMAX ACTIVATION
    esum = 0.0;
    for (i = 0; i < layerSizes[9]; i++)
    {
        sum = 0.0;
        for (j = 0; j < layerSizes[8] + 1; j++)
            sum += layers[8][j] * weights[9][i * (layerSizes[8] + 1) + j];
        layers[9][i] = exp(sum);
        if (layers[9][i] > 1e30)
            return -1; // GRADIENTS EXPLODED
        esum += layers[9][i];
    }

    // SOFTMAX FUNCTION
    max = layers[9][0];
    imax = 0;
    for (i = 0; i < layerSizes[9]; i++)
    {
        if (layers[9][i] > max)
        {
            max = layers[9][i];
            imax = i;
        }
        layers[9][i] = layers[9][i] / esum;
    }
    prob = layers[9][imax]; // ugly use of global variable :-(
    prob0 = layers[9][0];
    prob1 = layers[9][2];
    prob2 = layers[9][4];
    return imax;
}

/**********************************************************************/
/*      NEURAL NETWORK                                                */
/**********************************************************************/
float ReLU(float x)
{
    if (x > 0)
        return x;
    else
        return 0;
}

/**********************************************************************/
/*      NEURAL NETWORK                                                */
/**********************************************************************/
float TanH(float x)
{
    return 2.0 / (1.0 + exp(-2 * x)) - 1.0;
}
