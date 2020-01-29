# core stuff
import pandas as pd
import numpy  as np
import csv

# plotty stuff
import matplotlib.pyplot     as plt

# Just to not fuck things up
np.random.seed(42)

# CONSTANTS
DATA_PATH = "./data/"

MODE_DESCRIPTION = {
    'bin' : "Binomial CQIs",
    'uni' : "Uniform CQIs"
}

LAMBDA_DESCRIPTION = {
    'l01' : "Lambda = 0.1",
    'l09' : "Lambda = 0.9",
    'l13' : "Lambda = 1.3",
    'l2'  : "Lambda = 2.0",
    'l5'  : "Lambda = 5.0"
}

MODE_PATH = {
    'bin' : "binomial/",
    'uni' : "uniform/"
}

LAMBDA_PATH = {
    'l01' : "lambda01/",
    'l09' : "lambda09/",
    'l13' : "lambda13/",
    'l2'  : "lambda2/",
    'l5'  : "lambda5/"
}

CSV_PATH = {
    'sca' : "sca_res.csv",
    'vec' : "vec_res.csv"
}


def vector_parse(cqi, pkt_lambda):
    path_csv = DATA_PATH + MODE_PATH[cqi] + LAMBDA_PATH[pkt_lambda] + CSV_PATH['vec']
    data = pd.read_csv(path_csv, delimiter=",", quoting=csv.QUOTE_NONE, encoding='utf-8')
    
    clean_data = data[['run', 'vecvalue', 'type', 'name']]

    clean_data = clean_data[clean_data.type == 'vector']
    clean_data.reset_index(inplace=True, drop=True)

    # fix values...
    clean_data.name     = clean_data.name.apply(lambda x: x.split(':')[0])
    clean_data.vecvalue = clean_data.vecvalue.apply(lambda x: np.array([float(i) for i in x.replace('"', '').split(' ')]))
    
    # compute aggvalues for each iteration
    clean_data['mean'] = clean_data.vecvalue.apply(lambda x: x.mean())
    clean_data['max'] = clean_data.vecvalue.apply(lambda x: x.max())
    clean_data['min'] = clean_data.vecvalue.apply(lambda x: x.min())
    clean_data['std'] = clean_data.vecvalue.apply(lambda x: x.std())

    # rename vecvalue for simplicity...
    clean_data = clean_data.rename({'vecvalue':'value'}, axis=1)
    return clean_data[['run', 'name', 'value', 'mean', 'max', 'min', 'std']]


# Parse CSV file
def scalar_parse(cqi, pkt_lambda):
    path_csv = DATA_PATH + MODE_PATH[cqi] + LAMBDA_PATH[pkt_lambda] + CSV_PATH['sca']
    data = pd.read_csv(path_csv, usecols=['run', 'type', 'name', 'value'])
    
    # remove useless rows (first 100-ish rows)
    data = data[data.type == 'scalar']
    data.reset_index(inplace=True, drop=True)

    # clean name value
    data.name = data.name.apply(lambda x: x.split(':')[0])
    return data[['run', 'name', 'value']]


def describe_attribute(data, name):
    # print brief summary of attribute name (with percentiles and stuff)
    print(data[data.name == name].describe(percentiles=[.25, .50, .75, .95]))
    return


def gini(data):
    sorted_list = sorted(data)
    height, area = 0, 0
    for value in sorted_list:
        height += value
        area += height - value / 2.
    fair_area = height * len(data) / 2.
    return (fair_area - area) / fair_area


def lorenz_curve(data, attribute, value='value'):
    # prepare the plot
    selected_ds = data[data.name == attribute]
    plot_lorenz_curve(selected_ds[value].to_numpy())
    
    # prettify the plot
    plt.plot([0, 1], [0, 1], 'k')
    plt.title("Lorenz Curve for " + attribute)
    plt.show()
    return


def lorenz_curve_vec(data, attribute):
    # consider only the values for attribute
    clean_data = data[data.name == attribute]

    # for each iteration
    for i in range(0, len(clean_data)):
        # sort the data
        vec = clean_data.value.iloc[i]
        plot_lorenz_curve(vec)
    
    plt.plot([0, 1], [0, 1], 'k')
    plt.title("Lorenz Curve for " + attribute)
    plt.show()
    return


def plot_lorenz_curve(data):
    # sort the data
    sorted_data = np.sort(data)
    print(sorted_data)

    # compute required stuff
    n = sorted_data.size
    T = sorted_data.sum()
    x = [i/n for i in range(0, n+1)]
    y = sorted_data.cumsum()/T 
    print(y)
    y = np.hstack((0, y)) # add y=0

    # plot
    plt.plot(x, y)
    return


def ecdf(data, attribute, value='value', show=True):
    selected_ds = data[data.name == attribute]

    plot_ecdf(selected_ds[value].to_numpy())
    plt.title("ECDF for " + attribute)
    
    if show:
        plt.show()
    return


def plot_ecdf(data):
    sorted_data = np.sort(data)
    F_x = []
    n = sorted_data.size

    for x in sorted_data:
        s = 0
        for i in range(0, n):
            s = s + 1 if sorted_data[i] < x else s
        F_x.append(s/n)
    
    plt.plot(sorted_data, F_x)
    return


def plot_ecdf_vec(data, attribute, iteration=0, sample_size=1000, replace=False):
    # consider only what i need
    sample = data[data.name == attribute]
    sample = sample.value.iloc[iteration]

    # consider a sample
    if sample_size != None:
        sample = sample[np.random.choice(sample.shape[0], sample_size, replace=replace)]
    
    plot_ecdf(sample)
    plt.title("ECDF for " + attribute)
    plt.show()
    return



def check_iid(data, attribute):
    samples = data[data.name == attribute].value
    pd.plotting.lag_plot(samples)
    plt.title("Lag-Plot for " + attribute)
    plt.show()

    pd.plotting.autocorrelation_plot(samples)
    plt.title("Autocorrelation plot for " + attribute)
    plt.show()
    return
    


def scalar_analysis(cqi_mode, pkt_lambda, verbose=0):
    # parse csv
    print("* * * Scalar analysis: ")
    print("+ - - - - - - - - - - - - - - - - - - - - - - - - - -")
    print("|  * CQI Generation : " + MODE_DESCRIPTION[cqi_mode])
    print("|  * Exponential packet interarrival : " + LAMBDA_DESCRIPTION[pkt_lambda])
    print("+ - - - - - - - - - - - - - - - - - - - - - - - - - -\n\n")

    clean_data = scalar_parse(cqi_mode, pkt_lambda)
    
    if(verbose > 1):
        print("Clean dataset for lambda2-scalar")
        print(clean_data.head())

    if(verbose > 0):
        print("** Info about mean throughput: ")
        describe_attribute(clean_data, 'throughput')

        print("** Info about mean response time: ")
        describe_attribute(clean_data, 'responseTime')
        
        print("** Info about mean num served user (throughput 2): ")
        describe_attribute(clean_data, 'NumServedUser')

    # check iid
    if(verbose > 0):
        check_iid(clean_data, 'responseTime')
        check_iid(clean_data, 'throughput')
        check_iid(clean_data, 'NumServedUser')

    # plot lorenz curve for response time
    lorenz_curve(clean_data, 'responseTime')

    # end of analysis
    return


def all_ecdf(ds_list, attribute, labels=None):
    for ds in ds_list:
        ecdf(ds, attribute, show=False)

    plt.title("ECDF for " + attribute)    
    if labels:
        plt.legend(labels)
    plt.show()
    return


def load_all_bin():
    return [scalar_parse('bin', 'l13'),
            scalar_parse('bin', 'l2'),
            scalar_parse('bin', 'l5')]


def load_all_uni():
    return [scalar_parse('uni', 'l09'),
            scalar_parse('uni', 'l2'),
            scalar_parse('uni', 'l5')]


def main():
    print("\n\nPerformance Evaluation - Python Data Analysis\n")
    
    # VECTOR ANALYSIS
    clean_data = vector_parse('bin', 'l13')
    print(clean_data.head(100))

    # Lorenz curve...
    lorenz_curve_vec(clean_data, 'responseTime')

    # plot_ecdf_vec(clean_data, 'responseTime', sample_size=None)
    
    ###############################################

    # SCALAR ANALYSIS (USELESS????)

    # clean_data = scalar_parse('bin', 'l2')
    # lorenz_curve(clean_data, 'responseTime')

    # load all datasets of type UNIFORM
    # ds_uni = load_all_uni()
    # ds_bin = load_all_bin()

    # attr = ['throughput', 'responseTime', 'NumServedUser']


    # for ds in ds_uni:
    #     print("\nUNIFORM (l13, l2, l5)")
    #     for a in attr:
    #         print("INFO ABOUT " + a )
    #         describe_attribute(ds, a)
    #         print("****")
    #     print("\n\n")
    
    # for ds in ds_uni:
    #     print("\n\n\nBINOMIAL (l09, l2, l5)")
    #     for a in attr:
    #         print("INFO ABOUT " + a )
    #         describe_attribute(ds, a)
    #         print("****")
    #     print("\n\n")
    
    

    # print all ecdf all together  
    # all_ecdf(ds_bin, 
    #         attribute='responseTime',
    #         labels=['L = 0.9', 'L = 2.0', 'L = 5.0'])

    # end
    return


if __name__ == '__main__':
    main()