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

####################################################
#                       UTIL                       #
####################################################

def running_avg(x):
    return np.cumsum(x) / np.arange(1, x.size + 1)

###########
#TODO: DON'T USE THE FOLLOWING FUNCTIONS !!!!!!!!!!!
###########

def avg_vector(data, attribute, start=0, duration=None):
    # get the data....
    sel = data[data.name == attribute]
    avg_vector = []

    for row in sel.itertuples():
        ravg = running_avg(row.value);
        avg_vector.append(ravg)
    return avg_vector

#TODO: i don't know if this is really the avg vector, i think this is the 
# average vector only if there is just one vector 
# IT SHOULD BE TESTED!!!!!!!!
def plot_avg_vector(data, attribute, start=0, duration=None, warmup=0):
    # all the vectors have the same duration.... so np
    duration = data['time'].iloc[0].max() if duration is None else duration
    
    # get the data....
    sel = data[data.name == attribute]
    for row in sel.itertuples():
        plt.plot(row.time, running_avg(row.value))
    
    # plot the data
    plt.xlim(start+warmup, duration)
    plt.show()
    return


####################################################
#                       UTIL                       #
####################################################


####################################################
#                      PARSER                      #
####################################################

def parse_if_number(s):
    try: return float(s)
    except: return True if s=="true" else False if s=="false" else s if s else None


def parse_ndarray(s):
    return np.fromstring(s, sep=' ') if s else None


def parse_name_attr(s):
    return s.split(':')[0] if s else None


def parse_run(s):
    return int(s.split('-')[1]) if s else None
 

def vector_parse(cqi, pkt_lambda):
    path_csv = DATA_PATH + MODE_PATH[cqi] + LAMBDA_PATH[pkt_lambda] + CSV_PATH['vec']
    
    # vec files are huge, try to reduce their size ASAP!!
    data = pd.read_csv(path_csv, 
        delimiter=",", quoting=csv.QUOTE_NONNUMERIC, encoding='utf-8',
        usecols=['run', 'type', 'module', 'name', 'vecvalue', 'vectime'],
        converters = {
            'run'      : parse_run,
            'vectime'  : parse_ndarray, # i guess
            'vecvalue' : parse_ndarray,
            'name'     : parse_name_attr
        }
    )

    # remove useless rows
    data = data[data.type == 'vector']
    data.reset_index(inplace=True, drop=True)

    # compute aggvalues for each iteration
    data['mean'] = data.vecvalue.apply(lambda x: x.mean())
    data['max']  = data.vecvalue.apply(lambda x: x.max())
    data['min']  = data.vecvalue.apply(lambda x: x.min())
    data['std']  = data.vecvalue.apply(lambda x: x.std())

    # TODO: Maybe this one can be removed in the future when we know
    # for sure if all the timevec have the same duration (which is very
    # likely to be honest)
    data['duration']  = data.vectime.apply(lambda x: x.max())


    # rename vecvalue for simplicity...
    data = data.rename({'vecvalue':'value', 'vectime':'time'}, axis=1)
    return data[['run', 'name', 'time', 'value', 'mean', 'max', 'min', 'std', 'duration']].sort_values(['run', 'name'])


# Parse CSV file
def scalar_parse(cqi, pkt_lambda):
    path_csv = DATA_PATH + MODE_PATH[cqi] + LAMBDA_PATH[pkt_lambda] + CSV_PATH['sca']
    data = pd.read_csv(path_csv, 
        usecols=['run', 'type', 'name', 'value'],
        converters = {
            'run'  : parse_run,   
            'name' : parse_name_attr
        }
    )
    
    # remove useless rows (first 100-ish rows)
    data = data[data.type == 'scalar']
    data.reset_index(inplace=True, drop=True)
    return data[['run', 'name', 'value']].sort_values(['run', 'name'])


def describe_attribute_sca(data, name, value='value'):
    # print brief summary of attribute name (with percentiles and stuff)
    print(data[data.name == name][value].describe(percentiles=[.25, .50, .75, .95]))
    return


def describe_attribute_vec(data, name, iteration=0):
    values = pd.Series(data[data.name == name].value.iloc[iteration])
    print(values.describe(percentiles=[.25, .50, .75, .95]))
    return


def aggregate_responsetime(data, numIterations=10, numUsers=10):
    sel = data[data.attribute.starts_with("responseTime-")]
    
    rsp_matrix = pd.DataFrame()
    stats      = pd.DataFrame()

    for i in range(0, numUsers):
        # get the data
        tmp = sel[sel.attribute == ("responseTime-"+str(i))].value.mean()
        
        # eval the stats (idk if this is actually useful)
        stats['user-'+str(i)]['mean'] = rsp_matrix['user-'+str(i)].mean()
        stats['user-'+str(i)]['max']  = rsp_matrix['user-'+str(i)].max()
        stats['user-'+str(i)]['min']  = rsp_matrix['user-'+str(i)].min()


    return rsp_matrix, stats


####################################################
#                      PARSER                      #
####################################################


####################################################
#                      LORENZ                      #
####################################################

def gini(data):
    sorted_list = sorted(data)
    height, area = 0, 0
    for value in sorted_list:
        height += value
        area += height - value / 2.
    fair_area = height * len(data) / 2.
    return (fair_area - area) / fair_area


def lorenz_curve_sca(data, attribute, value='value'):
    # prepare the plot
    selected_ds = data[data.name == attribute]
    plot_lorenz_curve(selected_ds[value].to_numpy())
    
    # prettify the plot
    plt.plot([0, 1], [0, 1], 'k')
    plt.title("Lorenz Curve for " + attribute + " - " + value)
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

    # compute required stuff
    n = sorted_data.size
    T = sorted_data.sum()
    x = [i/n for i in range(0, n+1)]
    y = sorted_data.cumsum()/T 
    y = np.hstack((0, y))

    # plot
    plt.plot(x, y)
    return

####################################################
#                      LORENZ                      #
####################################################


####################################################
#                      ECDF                        #
####################################################

def all_ecdf(ds_list, attribute, labels=None):
    for ds in ds_list:
        ecdf_sca(ds, attribute, show=False)

    plt.title("ECDF for " + attribute)    
    if labels:
        plt.legend(labels)
    plt.show()
    return


def ecdf_sca(data, attribute, value='value', show=True):
    selected_ds = data[data.name == attribute]

    plot_ecdf(selected_ds[value].to_numpy())
    plt.title("ECDF for " + attribute)
    
    if show:
        plt.show()
    return


def plot_ecdf(data):
    # sort the values
    sorted_data = np.sort(data)
    
    # eval y
    n = sorted_data.size
    F_x = [(sorted_data[sorted_data <= x].size)/n for x in sorted_data]

    # plot the plot
    plt.plot(sorted_data, F_x)
    return


def plot_ecdf_vec(data, attribute, iteration=0, sample_size=1000, replace=False):
    # consider only what i need
    sample = data[data.name == attribute]
    sample = sample.value.iloc[iteration]

    # consider a sample
    if sample_size is not None:
        sample = sample[np.random.choice(sample.shape[0], sample_size, replace=replace)]
    
    plot_ecdf(sample)
    plt.title("ECDF for " + attribute)
    plt.show()
    return

####################################################
#                      ECDF                        #
####################################################


####################################################
#                      IID                         #
####################################################

def check_iid_sca(data, attribute, value='value'):
    samples = data[data.name == attribute][value]
    check_iid(samples, attribute)
    return


def check_iid_vec(data, attribute, iteration=0, sample_size=1000, seed=42):
    samples = pd.Series(data[data.name == attribute].value.iloc[iteration])

    # consider a sample
    if sample_size != None:
        samples = samples.sample(n=sample_size, random_state=seed)

    check_iid(samples, attribute)
    return


def check_iid(samples, attribute):
    pd.plotting.lag_plot(samples)
    plt.title("Lag-Plot for " + attribute)
    plt.show()

    pd.plotting.autocorrelation_plot(samples)
    plt.title("Autocorrelation plot for " + attribute)
    plt.show()
    return

####################################################
#                      IID                         #
####################################################

####################################################
####################################################
####################################################


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
        describe_attribute_sca(clean_data, 'throughput')

        print("** Info about mean response time: ")
        describe_attribute_sca(clean_data, 'responseTime')
        
        print("** Info about mean num served user (throughput 2): ")
        describe_attribute_sca(clean_data, 'NumServedUser')

    # check iid
    if(verbose > 0):
        check_iid_sca(clean_data, 'responseTime')
        check_iid_sca(clean_data, 'throughput')
        check_iid_sca(clean_data, 'NumServedUser')

    # plot lorenz curve for response time
    lorenz_curve_sca(clean_data, 'responseTime')

    # end of analysis
    return


def load_all_bin():
    return [scalar_parse('bin', 'l13'),
            scalar_parse('bin', 'l2'),
            scalar_parse('bin', 'l5')]


def load_all_uni():
    return [scalar_parse('uni', 'l09'),
            scalar_parse('uni', 'l2'),
            scalar_parse('uni', 'l5')]


####################################################
####################################################
####################################################


def main():
    print("\n\nPerformance Evaluation - Python Data Analysis\n")
    
    # VECTOR ANALYSIS
    clean_data = vector_parse('uni', 'l09')

    # preamble
    print(clean_data.head(100))
    # plot_avg_vector(clean_data, "responseTimeGlobal")
    plot_avg_vector(clean_data, "responseTime-0")

    return 


    # check_iid_vec(clean_data, 'responseTime')
    # for it in range(0, 100):
    #    describe_attribute_vec(clean_data, 'responseTime', iteration=it)

    # describe_attribute_sca(clean_data, 'throughput', value='max')
    # lorenz_curve_sca(clean_data, 'responseTime', value='max')

    # some analysis
    # lorenz_curve_vec(clean_data, 'responseTime')
    # plot_ecdf_vec(clean_data, 'responseTime', iteration=0, sample_size=1000)
    # check_iid_vec(clean_data, 'responseTime', iteration=0, sample_size=1000)

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
    #         describe_attribute_sca(ds, a)
    #         print("****")
    #     print("\n\n")
    
    # for ds in ds_uni:
    #     print("\n\n\nBINOMIAL (l09, l2, l5)")
    #     for a in attr:
    #         print("INFO ABOUT " + a )
    #         describe_attribute_sca(ds, a)
    #         print("****")
    #     print("\n\n")
    
    

    # print all ecdf all together  
    # all_ecdf(ds_bin, 
    #         attribute='responseTime',
    #         labels=['L = 0.9', 'L = 2.0', 'L = 5.0'])

    # end

if __name__ == '__main__':
    main()