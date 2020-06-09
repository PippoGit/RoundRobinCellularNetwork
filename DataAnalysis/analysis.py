# core stuff
import pandas as pd
import numpy  as np
import csv
import math

# scipy whatever
import scipy
from scipy import stats
import statsmodels.api as sm
from statsmodels.stats.anova import anova_lm

from statsmodels.distributions.empirical_distribution import ECDF, _conf_set

import uncertainties
from uncertainties import unumpy

# plotty stuff
import matplotlib.pyplot as plt
import seaborn as sns
sns.set(style="darkgrid")


# CONSTANTS
WARMUP_PERIOD  =      3
NUM_ITERATIONS =    100
SIM_TIME       =     60  
NUM_USERS      =     10
TIMESLOT       =  0.001

SAMPLE_SIZE    = 1000 # not really used
SEED_SAMPLING  =   42 # not really used

# DATA PATHs
DATA_PATH = "./data/"

MODE_DESCRIPTION = {
    'bin' : "Binomial CQIs",
    'uni' : "Uniform CQIs",
    'bin_old' : "Binomial CQIs (old)"
}

LAMBDA_DESCRIPTION = {
    'l01' : "1/λ = 0.1ms",
    'l02' : "1/λ = 0.2ms",
    'l07' : "1/λ = 0.7ms",
    'l09' : "1/λ = 0.9ms",
    'l1'  : "1/λ = 1.0ms",
    'l13' : "1/λ = 1.3ms",
    'l14' : "1/λ = 1.4ms",
    'l15' : "1/λ = 1.5ms",
    'l2'  : "1/λ = 2.0ms",
    'l25' : "1/λ = 2.5ms",
    'l5'  : "1/λ = 5.0ms"
}

MODE_PATH = {
    'bin' : "binomial/",
    'uni' : "uniform/",
    'bin_old' : "bin_old/"
}

LAMBDA_PATH = {
    'l01' : "lambda01/",
    'l02' : "lambda02/",
    'l07' : "lambda07/",
    'l09' : "lambda09/",
    'l1'  : "lambda1/",
    'l13' : "lambda13/",
    'l14' : "lambda14/",
    'l15' : "lambda15/",
    'l2'  : "lambda2/",
    'l25' : "lambda25/",
    'l5'  : "lambda5/"
}

CSV_PATH = {
    'sca' : "sca_res.csv",
    'vec' : "vec_res.csv"
}

CQI_CLASSES = [
    'LOW',
    'MID',
    'HIGH'
]

uni_lambdas=['l02', 'l07', 'l1', 'l15', 'l2', 'l25']


# Just to not fuck things up
np.random.seed(SEED_SAMPLING)


####################################################
#                       UTIL                       #
####################################################

def data_path(cqi, pkt_lambda, kind='sca'):
    return DATA_PATH + MODE_PATH[cqi] + LAMBDA_PATH[pkt_lambda] + CSV_PATH[kind]


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


def scatterplot_vector(data, attribute, start=WARMUP_PERIOD, duration=SIM_TIME, iterations=range(0, NUM_ITERATIONS), users=range(0, NUM_USERS)):
    sel = data[data.name.str.startswith(attribute + '-')]

    for u in users:
        usr = sel[sel.name == attribute + "-" + str(u)]
        for i in iterations:
            tmp = usr[(usr.run == i)]
            for row in tmp.itertuples():
                sns.scatterplot(row.time, row.value, label=str(u))
    
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
                plt.plot(row.time, running_avg(row.value), label=str(u))

    # plot the data
    plt.xlim(start, duration)
    plt.legend()
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
    return s.split(':')[1] + ':' + s.split(':')[0] if s else None


def parse_name_attr_stats(s):
    return s.split(':')[0] if s else None


def parse_run(s):
    return int(s.split('-')[1]) if s else None
 

def vector_parse(cqi, pkt_lambda):
    path_csv = DATA_PATH + MODE_PATH[cqi] + LAMBDA_PATH[pkt_lambda] + CSV_PATH['vec']
    return vector_parse_csv(path_csv)


def vector_parse_csv(path_csv):   
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

    # rename vecvalue for simplicity...
    data = data.rename({'vecvalue':'value', 'vectime':'time'}, axis=1)
    return data[['run', 'name', 'time', 'value']].sort_values(['run', 'name'])



# Parse CSV file
def scalar_parse(cqi, pkt_lambda, stats=True):
    path_csv = DATA_PATH + MODE_PATH[cqi] + LAMBDA_PATH[pkt_lambda] + CSV_PATH['sca']
    return scalar_parse_csv(path_csv, stats=stats)


def scalar_parse_csv(path_csv, stats=True):
    cols = ['run', 'name'] + (['count', 'mean', 'stddev', 'max', 'min'] if stats else ['value'])
    add_cols = ['sum', 'ci95', 'ci99']

    data = pd.read_csv(path_csv, 
        usecols=cols + ['type'],
        converters = {
            'run'  : parse_run,   
            'name' : parse_name_attr_stats if stats else parse_name_attr
        }
    )
    
    # remove useless rows (first 100-ish rows)
    data = data[data.type == 'statistic']
    data.reset_index(inplace=True, drop=True)
    
    # data_cln = data[data.value.isna()]    
    # data_sum_sig = data[data.value.notna()]
    # data_sum_sig.name = 'sum:' + data_sum_sig['name'].astype(str)
    # result = pd.concat([data_cln, data_sum_sig])

    data['ci99'] = 2.58*(data['stddev']/np.sqrt(data['count']))
    data['sum']  = data['count']*data['mean']
    data['ci95'] = 1.96*(data['stddev']/np.sqrt(data['count']))
    data['ci99'] = 2.58*(data['stddev']/np.sqrt(data['count']))

    # data['user'] = data.name.apply(lambda x: x.split['-'][1] if '-' in x else 'global')
    return data[cols + add_cols].sort_values(['run', 'name']).reset_index().drop(columns=['index'])


def describe_attribute_sca(data, name, value='mean'):
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


# TODO: FORSE VA RIFATTA, o si ignorano quegli aggregated stuff
def aggregate_users_signals(data, signal, users=range(0, NUM_USERS), value='mean'):
    # meanResponseTime => dato il mean response time di un utente per ogni run, calcolo la media dei
    # mean response time dell'utente su tutte le run. E poi faccio la media per tutti gli utenti per
    # ottenere il mean responsetime medio per tutti gli utenti.
    return data[data.name.isin([signal + '-' + str(i) for i in users])].groupby('run').mean().describe(percentiles=[.25, .50, .75, .95])

def scalar_stats(mode, lval, attributes=None, users=range(0,NUM_USERS), value='mean'):
    data = scalar_parse(mode, lval)
    stats = pd.DataFrame()
    attributes = data.name.unique() if attributes is None else attributes

    # STATS FOR EACH SIGNAL
    for attr in attributes: 
        stats[attr] = data[data.name == attr][value].describe(percentiles=[.25, .50, .75, .95])

    # Transpose...
    stats = stats.T

    # COMPUTE CI 
    stats['ci95_l'] = stats['mean'] - 1.96*(stats['std']/np.sqrt(stats['count']))
    stats['ci95_h'] = stats['mean'] + 1.96*(stats['std']/np.sqrt(stats['count']))
    stats['ci99_l'] = stats['mean'] - 2.58*(stats['std']/np.sqrt(stats['count']))
    stats['ci99_h'] = stats['mean'] + 2.58*(stats['std']/np.sqrt(stats['count']))
    return stats


def users_bandwidth_sca(data, group=False):
    stats = scalar_stats(data)
    index = [row for row in stats.index if row.startswith('tptUser-')]
    sel = stats.loc[index, :].reset_index()

    bandwidth = pd.DataFrame()
    bandwidth['user'] = sel['index'].str.split('-', expand=True)[1].astype(int)
    bandwidth['mean_Mbps'] = (sel['mean'])/125000
    bandwidth['max_Mbps']  = (sel['max'])/125000
    bandwidth['min_Mbps']  = (sel['min'])/125000
    
    bandwidth.index = bandwidth['user'] 
    bandwidth = bandwidth.drop('user', axis=1)
    return bandwidth


####################################################
#                      PARSER                      #
####################################################


####################################################
#                      LORENZ                      #
####################################################

def gini(data, precision=3):
    sorted_list = sorted(data)
    height, area = 0, 0
    for value in sorted_list:
        height += value
        area += height - value/2.
    fair_area = height * len(data)/2.
    return round((fair_area - area) / fair_area, precision)


def lorenz_curve_sca(data, attribute, users=range(0, NUM_USERS), iterations=range(0, NUM_ITERATIONS), value='mean'):
    # val = pd.DataFrame()
    sel = data[data.name.str.startswith(attribute + '-')]
    sel['user'] = sel.name.str.split('-', expand=True)[1].astype(int)
    sorted_data = pd.DataFrame()

    for r in iterations:
        tmp = sel[sel.run == r]
        sorted_data['run-' + str(r)] = np.sort(tmp[value].values)
    

    # return sorted_data
    plot_lorenz_curve(sorted_data.mean(axis=1))
    plt.plot([0, 1], [0, 1], 'k', alpha=0.85)
    plt.title("Lorenz Curve for " + value + " " + attribute + " -  Gini: " + str(gini(sorted_data.mean(axis=1))))
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


def plot_lorenz_curve(data, color=None, alpha=1):
    # sort the data
    sorted_data = np.sort(data)

    # compute required stuff
    n = sorted_data.size
    T = sorted_data.sum()
    x = [i/n for i in range(0, n+1)]
    y = sorted_data.cumsum()/T 
    y = np.hstack((0, y))

    # plot
    plt.plot(x, y, color=color, alpha=alpha)
    return


def all_lorenz(mode, lambda_val, attribute, users=range(0, NUM_USERS), iterations=range(0, NUM_ITERATIONS), save=False, value='mean'):
    data = scalar_parse(mode, lambda_val)

    # Plot the mean lorenz
    sel = data[data.name.str.startswith(attribute + '-')]
    sel['user'] = sel.name.str.split('-', expand=True)[1].astype(int)
    sorted_data = pd.DataFrame()

    for r in iterations:
        tmp = sel[sel.run == r]
        sorted_data['run-' + str(r)] = np.sort(tmp[value].values)
        plot_lorenz_curve(sorted_data['run-' + str(r)], color='grey', alpha=0.25)

    # return sorted_data
    plot_lorenz_curve(sorted_data.mean(axis=1))

    plt.plot([0, 1], [0, 1], 'k', alpha=0.85)
    plt.title(value + " " + attribute + ": " + MODE_DESCRIPTION[mode] + ' and ' + LAMBDA_DESCRIPTION[lambda_val] + ' - Mean Gini: ' + str(gini(sorted_data.mean(axis=1))))
    
    if save:
        plt.savefig("lorenz_responseTime_" + mode + "_" + lambda_val + ".pdf")
        plt.clf()
    else: 
        plt.show()
    
    return

####################################################
#                      LORENZ                      #
####################################################


####################################################
#                      ECDF                        #
####################################################


def multi_ecdf_sca(mode, lambdas, attribute, users=range(0, NUM_USERS), save=False, show='single', hide_users=False, hide_mean=False, value='mean'):
    for l in lambdas:
        data = scalar_parse(mode, l)
    
        if show is 'aggregate':
            x = pd.DataFrame()
            for u in users:
                stats = data[data.name == attribute + '-' + str(u)]
                x[str(u)] = stats[value].to_numpy()
                
                if hide_users is False:
                    ecdf = ECDF(x[str(u)])
                    sns.lineplot(x=ecdf.x, y=ecdf.y, drawstyle='steps-post', c='gray', alpha=0.7)
            
            if hide_mean is False:
                mean  = x.mean(axis=1)
                ecdf  = ECDF(mean.to_numpy())
                sns.lineplot(x=ecdf.x, y=ecdf.y,label="AVG User " + LAMBDA_DESCRIPTION[l], drawstyle='steps-post')
        
        elif show is 'class':
            x0 = data[data.name == attribute + '-0']
            x1 = data[data.name == attribute + '-1']

            ecdf0 = ECDF(x0[value])
            ecdf1 = ECDF(x1[value])
            sns.lineplot(x=ecdf0.x, y=ecdf0.y,label="Good User " + LAMBDA_DESCRIPTION[l], drawstyle='steps-post')
            sns.lineplot(x=ecdf1.x, y=ecdf1.y,label="Bad User " + LAMBDA_DESCRIPTION[l], drawstyle='steps-post')

        else:
            selected_ds = data[data.name == attribute]
            x = selected_ds[value].to_numpy()           
            ecdf = ECDF(x)
            plt.step(ecdf.x, ecdf.y, label=LAMBDA_DESCRIPTION[l], where='post')

    title = "ECDF (" + MODE_DESCRIPTION[mode] + ") for " + value + " " +  attribute + (" with " + str(len(users)) + " users" if show is 'aggregate' else "")
    plt.title(title)
    plt.legend(loc='upper center', bbox_to_anchor=(0.5, -0.05),
          fancybox=True, shadow=True, ncol=6)
    
    if save:
        plt.savefig("ecdf_" + attribute + ".pdf", bbox_inches="tight")
        plt.clf()
    else:
        plt.show()
    return 


def ecdf_sca(mode, lambda_val, attribute, value='mean', show='single', users=range(0, NUM_USERS), save=False):
    multi_ecdf_sca(mode, lambdas=[lambda_val], attribute=attribute, show=show, users=users,save=save, value=value)
    return


def plot_ecdf(data, lambda_val=None):
    # sort the values
    sorted_data = np.sort(data)
    
    # eval y
    n = sorted_data.size
    F_x = [(sorted_data[sorted_data <= x].size)/n for x in sorted_data]

    # plot the plot
    # sns.lineplot(x=sorted_data, y=F_x, label=lambda_val)
    plt.step(sorted_data, F_x, label=lambda_val)
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

def check_iid_sca(data, attribute, aggregate=False, users=range(0, NUM_USERS), save=False, value='mean'):
    if aggregate:
        samples = data[data.name.isin([attribute + '-' + str(i) for i in users])].groupby('run').mean()
    else:
        samples = data[data.name == attribute][value]
    check_iid(samples, attribute, aggregate=aggregate, save=save)
    return


def check_iid_vec(data, attribute, iteration=0, sample_size=1000, seed=42, save=False):
    samples = pd.Series(data[data.name == attribute].value.iloc[iteration])

    # consider a sample
    if sample_size != None:
        samples = samples.sample(n=sample_size, random_state=seed)

    check_iid(samples, attribute, save)
    return


def check_iid(samples, attribute, aggregate=False, save=False):
    pd.plotting.lag_plot(samples)
    plt.title("Lag-Plot for " + attribute + (" (mean) " if aggregate else ""))
    
    if aggregate:
        plt.ylim(samples.min().value - samples.std().value, samples.max().value + samples.std().value)
        plt.xlim(samples.min().value - samples.std().value, samples.max().value + samples.std().value)
    else:
        plt.ylim(samples.min() - samples.std(), samples.max() + samples.std())
        plt.xlim(samples.min() - samples.std(), samples.max() + samples.std())  

    if save:
        plt.savefig("iid_lagplot_" + attribute + ".pdf", bbox_inches="tight")
        plt.clf()
    else:
        plt.show()

    pd.plotting.autocorrelation_plot(samples)
    plt.title("Autocorrelation plot for " + attribute + (" (mean) " if aggregate else ""))
    if save:
        plt.savefig("iid_autocorrelation_" + attribute + ".pdf", bbox_inches="tight")
        plt.clf()
    else:
        plt.show()

    return

####################################################
#                      IID                         #
####################################################

####################################################
####################################################
####################################################




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
        'uni' : ['l09', 'l15', 'l2', 'l5'],
        'bin_old' : ['l14', 'l15', 'l2', 'l5'],
        'bin' : ['l15', 'l2', 'l5']
    }

    for m in exp.keys():
        for l in exp[m]:
            data = scalar_parse(m, l)
            stats = scalar_stats(data)
            stats.to_csv('stats_' + m + '_' + l + '.csv')
    return



def unibin_ci_plot(lambda_val, attr, bin_mode='bin', ci=95, save=False):
    # get the data...
    stats1 = scalar_stats(scalar_parse('uni', lambda_val))
    stats2 = scalar_stats(scalar_parse(bin_mode, lambda_val))

    bar1 = stats1['mean'][attr]    
    bar2 = stats2['mean'][attr]
    
    error = np.array([bar1 - stats1['ci' + str(ci) + '_l'][attr], stats1['ci' + str(ci) + '_h'][attr] - bar1]).reshape(2,1)
    plt.bar(MODE_DESCRIPTION['uni'], bar1, yerr=error, align='center', ecolor='black', capsize=7)
    
    error = np.array([bar2 - stats2['ci' + str(ci) + '_l'][attr], stats2['ci' + str(ci) + '_h'][attr] - bar2]).reshape(2,1)
    plt.bar(MODE_DESCRIPTION[bin_mode], bar2, yerr=error, align='center', ecolor='black', capsize=7)
    
    # Show graphic
    plt.title("Comparison for " + attr + " and " + LAMBDA_DESCRIPTION[lambda_val])
    if save:
        plt.savefig("compare_unibin_"+ attr + "_" + lambda_val + ".pdf", bbox_inches="tight")
        plt.clf()
    else:
        plt.show()
    return


def plot_to_img(mode, lambdas):
    for l in lambdas:
        all_lorenz(mode, l, 'rspTimeUser', value='mean', save=True)
    return


# this function works only with "nameSignal-user" kind of statistics 
# (responseTime-#numuser or tptUser-#numuser)
def histo_users(mode, lambda_val, attribute, value='mean', ci="95", hue=None, title=None, palette=None):
    t = title if title else "Mean " + attribute + " per user, CI=" + ci + " (" + MODE_DESCRIPTION[mode] + ", " + LAMBDA_DESCRIPTION[lambda_val]  + ")"
    data = tidy_scalar(mode, lambda_val)

    grp = data.groupby(['user'])    
    md = grp.mean()
    err = md[attribute + ':ci'+ci]
    
    sns.catplot(x='user', y=attribute + ':' + value, data=data, kind='bar', capsize=0.6, errwidth=1.4, dodge=False, hue=hue, palette=palette, ci=int(ci))
    plt.title(t)
    plt.show()
    return


def histo_all_lambdas(mode, lambdas=['l02', 'l07', 'l1', 'l15', 'l2', 'l25'], attribute='rspTimeUser', value='mean', hue=None, title=None, palette=None, ci="95", antenna=False):
    t = title if title else "Mean " + attribute + " per workload, CI=" + ci + " (" + MODE_DESCRIPTION[mode] +  ")"

    multi_tidy = multi_tidy_scalar({mode:lambdas}, antenna=antenna)
    sns.catplot(x='lambda', y=attribute + ':' + value, data=multi_tidy, kind='bar', capsize=0.6, errwidth=1.4, dodge=False, hue=hue, palette=palette, ci=int(ci))
    plt.title(t)
    plt.show()
    return



def scatterplot_mean(mode, lambda_val, x_attr, y_attr, users=range(0, NUM_USERS), save=False, group=None, hue='user', col=None):
    data = tidy_scalar(mode, lambda_val)

    if group is not None:
        data = data.groupby(group).mean().reset_index()
        hue = group

    # cutie scatterplot
    sns.scatterplot(x=x_attr, y=y_attr, data=data, hue=hue)
    plt.title("Scatterplot " + x_attr + " - " + y_attr + " (" + MODE_DESCRIPTION[mode] + ", " + LAMBDA_DESCRIPTION[lambda_val]  + ")")
    plt.show()

    # kind of regression plot?
    sns.lmplot(x=x_attr, y=y_attr, data=data, col=col)
    plt.show()

    sns.lmplot(x=x_attr, y=y_attr, data=data, lowess=True)
    plt.show()

    sns.jointplot(x=x_attr, y=y_attr, data=data, kind="reg")
    plt.show()

    return


# does it make sense?
def CQI_to_class(cqi):
    if cqi < 4: return CQI_CLASSES[0]
    if cqi < 10: return CQI_CLASSES[1]
    return CQI_CLASSES[2]



def class_plot(mode, lambda_val, y_attr='rspTimeUser', value='mean', ci='95'):
    colors = [sns.xkcd_rgb["pale red"], sns.xkcd_rgb["denim blue"]]# sns.xkcd_rgb["medium green"], sns.xkcd_rgb["denim blue"]]
    histo_users('bin', 'l15', y_attr, hue='class', ci=ci, value='mean', palette=colors)
    return


# TIDY SCALAR CONTAINS ONLY USER SIGNALS!!!!!!!!!!!
def tidy_scalar_csv(path_to_csv):
    tidy_data = pd.DataFrame()
    data = scalar_parse_csv(path_to_csv)
    sel = data[data.name.str.contains('-')].reset_index().drop('index', axis=1)
    sel[['attr', 'user']] = sel.name.str.split('-', expand=True)
    sel = sel.drop('name', axis=1)

    tidy_data['user']  = 'user-' + sel[sel.attr == sel.attr.iloc[0]].user.values # any dynsignal will be fine, because all of them have 100 instances
    tidy_data['run']   = sel[sel.attr == sel.attr.iloc[0]].run.values # same
    for attr_name in sel.attr.unique():
        for val in ['sum', 'count', 'mean', 'min', 'max', 'stddev', 'ci95', 'ci99']:
            tidy_data[attr_name + ':' + val] = sel[sel.attr == attr_name][val].values
        # tidy_data[attr_name + ":ci95"] = 1.96*(tidy_data[attr_name + ':stddev']/np.sqrt(tidy_data[attr_name + ':count']))
        # tidy_data[attr_name + ":ci95_u"] = tidy_data[attr_name + ':mean'] + 1.96*(tidy_data[attr_name + ':stddev']/np.sqrt(tidy_data[attr_name + ':count']))
        # tidy_data[attr_name + ":ci99_l"] = tidy_data[attr_name + ':mean'] - 2.58*(tidy_data[attr_name + ':stddev']/np.sqrt(tidy_data[attr_name + ':count']))
        # tidy_data[attr_name + ":ci99"] = 2.58*(tidy_data[attr_name + ':stddev']/np.sqrt(tidy_data[attr_name + ':count']))

    tidy_data['class'] = tidy_data['CQIUser:mean'].apply(lambda x: CQI_to_class(x))
    return tidy_data


def tidy_scalar(mode, lambda_val, antenna=False):
    return tidy_scalar_antenna_csv(data_path(mode, lambda_val)) if antenna else tidy_scalar_csv(data_path(mode, lambda_val))


def tidy_scalar_antenna_csv(path_to_csv):
    tidy_data = pd.DataFrame()
    data = scalar_parse_csv(path_to_csv)
    sel = data[~data.name.str.contains('-')].reset_index().drop('index', axis=1)

    tidy_data['run']   = sel[sel.name == sel.name.iloc[0]].run.values # same
    for attr_name in sel.name.unique():
        for val in ['sum', 'count', 'mean', 'min', 'max', 'stddev', 'ci95', 'ci99']:
            tidy_data[attr_name + ':' + val] = sel[sel.name == attr_name][val].values
    return tidy_data


def model_validation(lambda_val, cqis=["2", "13"], attribute="mean:CQIUser", ci=95):
    data = {}
    for cqi in cqis:
        data[cqi] = pd.read_csv(DATA_PATH + "modelv/cqi" + cqi + lambda_val + ".csv", 
            usecols=['run', 'type', 'name', 'value'],
            converters = {
                'run'  : parse_run,   
                'name' : parse_name_attr
            }
        )
        
        # remove useless rows (first 100-ish rows)
        data[cqi] = data[cqi][data[cqi].type == 'scalar']
        data[cqi].reset_index(inplace=True, drop=True)
        data[cqi] = data[cqi][['run', 'name', 'value']].sort_values(['run', 'name'])

        stats = scalar_stats(data[cqi], users=[0])
        attr =  attribute + '-0'
        bar = stats['mean'][attr]
        error = np.array([bar - stats['ci' + str(ci) + '_l'][attr], stats['ci' + str(ci) + '_h'][attr] - bar]).reshape(2,1)
        plt.bar('CQI ' + cqi, bar, yerr=error, align='center', alpha=0.95, ecolor='k', capsize=7)
    
    plt.title(attribute + " - " + LAMBDA_DESCRIPTION[lambda_val] + ", CI="+ str(ci))
    plt.show()
    return data


def load_data_test():
    data = pd.read_csv(DATA_PATH + "modelv/cqi10l5.csv", 
        usecols=['run', 'type', 'name', 'value'],
        converters = {
            'run'  : parse_run,   
            'name' : parse_name_attr
        }
    )
    return data



def catplot(cat, y, mode='uni'):
    data = multi_tidy_scalar()
    sns.catplot(x=cat, y=y, jitter=False, data=data[data.cqi_mode == mode])
    plt.show()
    return


def correlation(x, y):
    data = multi_tidy_scalar()
    sns.scatterplot(x, y, data=data, hue='lambda', style='cqi_mode')
    plt.show()
    return


def plot_correlation_tpt_lambda(mode='uni', lambdas=['l02', 'l07', 'l1', 'l15', 'l2', 'l25'], value='mean'):
    data = multi_tidy_scalar({mode:lambdas})
    sns.scatterplot('tptUser' + ':' +  value,'lambda',  data=data, hue='lambda')
    plt.show()
    return data



def multi_tidy_scalar(lambdas={'uni': ['l02', 'l07', 'l1', 'l15'], 'bin': ['l07', 'l1', 'l15']}, antenna=False):
    x = pd.DataFrame()
    for k in lambdas.keys():
        for l in lambdas[k]:
            y = tidy_scalar(k, l, antenna=antenna)
            y['lambda'] = LAMBDA_DESCRIPTION[l]
            y['cqi_mode'] = MODE_DESCRIPTION[k]
            x = x.append(y, ignore_index=True)
    return x



def test_ecdf_vect(data, attribute):
    stats = data[data.name == attribute]
    a = np.array([])

    for row in stats.itertuples():
        a = np.append(a, row.value)
    
    ecdf = ECDF(a)
    sns.lineplot(x=ecdf.x, y=ecdf.y, drawstyle='steps-post')
    plt.show()
    return


def main():
    lambdas = ['l02', 'l1', 'l15']


    print("\n\nPerformance Evaluation - Python Data Analysis\n")
    print("*" * 30, "\n")

    print("Testing UNIFORM" )
    for l in lambdas:
        print("LAMBDA VALUE: " + LAMBDA_DESCRIPTION[l])
        data = scalar_parse('uni', l)
        tidy = tidy_scalar('uni', l)
        
        print("Scalar Stats: ")
        print(scalar_stats(data).to_string())
        
        print("Checking IID for aggregated mean responseTime")
        check_iid_sca(data, 'mean:rspTimeUser', aggregate=True)

        print("Checking IID for aggregated mean throughput")
        check_iid_sca(data, 'mean:tptUser', aggregate=True)



        input("Press enter to continue...")
        

    



    return 

if __name__ == '__main__':
    main()