# core stuff
import pandas as pd
import numpy  as np
from scipy import stats
import csv

# plotty stuff
import matplotlib.pyplot as plt

# CONSTANTS
WARMUP_PERIOD  = 70
NUM_ITERATIONS = 100
SIM_TIME       = 200
NUM_USERS      = 10

SAMPLE_SIZE    = 1000
SEED_SAMPLING  = 42

# DATA PATHs
DATA_PATH = "./data/"

MODE_DESCRIPTION = {
    'bin' : "Binomial CQIs",
    'uni' : "Uniform CQIs"
}

LAMBDA_DESCRIPTION = {
    'l01' : "Lambda = 0.1",
    'l09' : "Lambda = 0.9",
    'l1'  : "Lambda = 1",
    'l13' : "Lambda = 1.3",
    'l14' : "Lambda = 1.4",
    'l15' : "Lambda = 1.5",
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
    'l1'  : "lambda1/",
    'l13' : "lambda13/",
    'l14' : "lambda14/",
    'l15' : "lambda15/",
    'l2'  : "lambda2/",
    'l5'  : "lambda5/"
}

CSV_PATH = {
    'sca' : "sca_res.csv",
    'vec' : "vec_res.csv"
}

# Just to not fuck things up
np.random.seed(SEED_SAMPLING)


def one_iter_lorenz(data, attribute, iteration=0):
    # consider only the values for attribute
    clean_data = data[data.name == attribute]

    # for each iteration
    # for i in range(0, len(clean_data)):
    # sort the data
    vec = clean_data.value.iloc[iteration]
    plot_lorenz_curve(vec)
    
    plt.plot([0, 1], [0, 1], 'k')
    plt.title("Lorenz Curve for " + attribute)
    plt.show()
    return


####################################################
#                       UTIL                       #
####################################################

def running_avg(x):
    return np.cumsum(x) / np.arange(1, x.size + 1)


def winavg(x, N):
    xpad = np.concatenate((np.zeros(N), x)) # pad with zeroes
    s = np.cumsum(xpad)
    ss = s[N:] - s[:-N]
    ss[N-1:] /= N
    ss[:N-1] /= np.arange(1, min(N-1,ss.size)+1)
    return ss


def filter_data(data, attribute, start=0):
    sel = data[data.name == attribute]

    for i, row in sel.iterrows():
        tmp = np.where(row.time < start, np.nan, row.value)
        sel.at[i, 'value'] = tmp[~np.isnan(tmp)]
    return sel



def plot_mean_vectors(data, attribute, start=0, duration=SIM_TIME, iterations=[0]):
    sel = data[data.name == attribute]    
    
    # plot a mean vector for each iteration
    for i in iterations:
        tmp = sel[sel.run == i]
        for row in tmp.itertuples():
            plt.plot(row.time, running_avg(row.value))
    
    # plot the data
    plt.xlim(start, duration)
    plt.show()
    return


def plot_mean_vectors_user(data, prefix, start=0, duration=SIM_TIME, iterations=[0], users=range(0, NUM_USERS)):
    sel = data[data.name.str.startswith(prefix + '-')]

    for u in users:
        usr = sel[sel.name == prefix + "-" + str(u)]
        for i in iterations:
            tmp = usr[(usr.run == i)]
            for row in tmp.itertuples():
                plt.plot(row.time, running_avg(row.value))

    # plot the data
    plt.xlim(start, duration)
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

    # # compute aggvalues for each iteration
    # data['mean']  = data.vecvalue.apply(lambda x: x.mean())
    # data['max']   = data.vecvalue.apply(lambda x: x.max())
    # data['min']   = data.vecvalue.apply(lambda x: x.min())
    # data['std']   = data.vecvalue.apply(lambda x: x.std())
    # data['count'] = data.vecvalue.apply(lambda x: x.size)

    # rename vecvalue for simplicity...
    data = data.rename({'vecvalue':'value', 'vectime':'time'}, axis=1)
    return data[['run', 'name', 'time', 'value']].sort_values(['run', 'name'])


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
    
    # data['user'] = data.name.apply(lambda x: x.split['-'][1] if '-' in x else 'global')
    
    return data[['run', 'name', 'value']].sort_values(['run', 'name'])


def describe_attribute_sca(data, name, value='value'):
    # print brief summary of attribute name (with percentiles and stuff)
    print(data[data.name == name][value].describe(percentiles=[.25, .50, .75, .95]))
    return


def describe_attribute_vec(data, name, iteration=0):
    values = pd.Series(data[data.name == name].value.iloc[iteration])
    print(values.describe(percentiles=[.25, .50, .75, .95]))
    return


def vector_stats(data, group=False):
    # compute stats for each iteration
    stats = pd.DataFrame()
    stats['name']  = data.name
    stats['run']   = data.run
    stats['mean']  = data.value.apply(lambda x: x.mean())
    stats['max']   = data.value.apply(lambda x: x.max())
    stats['min']   = data.value.apply(lambda x: x.min())
    stats['std']   = data.value.apply(lambda x: x.std())
    stats['count'] = data.value.apply(lambda x: x.size)
    return stats.groupby(['name']).mean().drop('run', axis=1) if group else stats


def scalar_stats(data, attr=None):
    stats = pd.DataFrame()
    attributes = data.name.unique() if attr is None else attr

    # STATS FOR EACH SIGNAL
    for attr in attributes: 
        stats[attr] = data[data.name == attr].value.describe(percentiles=[.25, .50, .75, .95])

    # MEAN-STATS:
    # ALTERNATIVA 2:
    stats['meanResponseTime'] = data[data.name.isin(['responseTime-' + str(i) for i in range(0, 10)])].groupby('run').mean().describe(percentiles=[.25, .50, .75, .95])
    stats['meanThroughput']   = data[data.name.isin(['tptUser-' + str(i) for i in range(0, 10)])].groupby('run').mean().describe(percentiles=[.25, .50, .75, .95])

    # ALTERNATIVA 1:
    # stats['meanResponseTime'] = stats[['responseTime-' + str(i) for i in range(0, NUM_USERS)]].T['mean'].describe(percentiles=[.25, .50, .75, .95])
    # stats['meanThroughput']   = stats[['tptUser-' + str(i) for i in range(0, NUM_USERS)]].T['mean'].describe(percentiles=[.25, .50, .75, .95])
    
    # Transpose...
    stats = stats.T

    # COMPUTE CI
    stats['ci95_l'] = stats['mean'] - 1.96*(stats['std']/np.sqrt(stats['count']))
    stats['ci95_h'] = stats['mean'] + 1.96*(stats['std']/np.sqrt(stats['count']))
    stats['ci99_l'] = stats['mean'] - 2.58*(stats['std']/np.sqrt(stats['count']))
    stats['ci99_h'] = stats['mean'] + 2.58*(stats['std']/np.sqrt(stats['count']))

    return stats


def users_bandwidth(data, group=False):
    sel = data[data.name.str.startswith('tptUser')]
    stats = vector_stats(sel)
    
    stats['user'] = sel.name.apply(lambda x: int(x.split('-')[1]))
    stats['run']  = sel.run

    stats['mean_Mbps'] = (stats['mean'] * 1000)/125000
    stats['max_Mbps']  = (stats['max']  * 1000)/125000
    stats['min_Mbps']  = (stats['min']  * 1000)/125000

    stats = stats[['user', 'run', 'mean_Mbps', 'max_Mbps', 'min_Mbps']]
    return stats.groupby(['user']).mean().drop('run', axis=1) if group else stats


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
        area += height - value/2.
    fair_area = height * len(data)/2.
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
    plot_mean_vectors(clean_data, "responseTime-0", start=WARMUP_PERIOD)

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



def test_lorenz(data, attribute, users=range(0, NUM_USERS), iterations=range(0, NUM_ITERATIONS)):
    # val = pd.DataFrame()
    sel = data[data.name.str.startswith(attribute + '-')]
    sel['user'] = sel.name.apply(lambda x:int(x.split('-')[1]))
    sorted_data = pd.DataFrame()

    for r in iterations:
        tmp = sel[sel.run == r]
        sorted_data['run-' + str(r)] = np.sort(tmp.value.values)
    
    print(sel.head(100))
    print(sorted_data.head(100))

    # return sorted_data
    plot_lorenz_curve(sorted_data.mean(axis=1))
    plt.show()
    return



def plot_winavg_vectors(data, attribute, start=0, duration=100, iterations=[0], win=100):
    sel = data[data.name == attribute]    
    
    # plot a mean vector for each iteration
    for i in iterations:
        tmp = sel[sel.run == i]
        for row in tmp.itertuples():
            plt.plot(row.time, winavg(row.value, win))
    
    # plot the data
    plt.xlim(start, duration)
    plt.show()
    return






def stats_to_csv():
    exp = {
        'uni' : ['l09', 'l2', 'l5'],
        'bin' : ['l14', 'l15', 'l2', 'l5']
    }

    for m in exp.keys():
        for l in exp[m]:
            data = scalar_parse(m, l)
            stats = scalar_stats(data)
            stats.to_csv('stats_' + m + '_' + l + '.csv')

    return



def ci_plot(lambda_val, ci, attr):
    # get the data...
    stats1 = scalar_stats(scalar_parse('uni', lambda_val))
    stats2 = scalar_stats(scalar_parse('bin', lambda_val))

    bar1 = stats1['mean'][attr]    
    bar2 = stats2['mean'][attr]
    
    error = np.array([bar1 - stats1['ci' + str(ci) + '_l'][attr], stats1['ci' + str(ci) + '_h'][attr] - bar1]).reshape(2,1)
    plt.bar('uni', bar1, yerr=error, align='center', alpha=0.5, ecolor='black', capsize=10)
    
    error = np.array([bar2 - stats2['ci' + str(ci) + '_l'][attr], stats2['ci' + str(ci) + '_h'][attr] - bar2]).reshape(2,1)
    plt.bar('bin', bar2, yerr=error, align='center', alpha=0.5, ecolor='black', capsize=10)
    
    # Show graphic
    plt.show()
